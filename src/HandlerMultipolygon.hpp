#ifndef OSMIUM_HANDLER_MULTIPOLYGON_HPP
#define OSMIUM_HANDLER_MULTIPOLYGON_HPP

#include <google/sparse_hash_map>

#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>

namespace Osmium {

    namespace Handler {

        class Multipolygon : public Base {

            /// a list of multipolygons that need to be completed
            std::vector<Osmium::OSM::MultipolygonFromRelation *> multipolygons;

            // a map from way_id to a vector of indexes into the multipolygons array
            // this is used to find in which multipolygon relations a way is
            typedef google::sparse_hash_map<osm_object_id_t, std::vector<osm_object_id_t> > way2mpidx_t;
            way2mpidx_t way2mpidx;

            struct callbacks *cb;

          public:

            Multipolygon(struct callbacks *cb) : cb(cb) {
            }

            // in pass 2
            void callback_before_ways() {
                Osmium::OSM::Object::init(); // initialize geos lib
            }

            // in pass 1
            void callback_relation(OSM::Relation *relation) {
                const char *type = relation->get_tag_by_key("type");
                if (!type) { return; }

                bool is_boundary;
                if (strcmp(type, "multipolygon") == 0) {
                    is_boundary = false;
                } else if (strcmp(type, "boundary") == 0) {
                    is_boundary = true;
                } else {
                    return;
                }

                Osmium::OSM::Relation *r = new Osmium::OSM::Relation(*relation);

                int num_ways = 0;
                for (int i=0; i < relation->member_count(); i++) {
                    Osmium::OSM::RelationMember *member = relation->get_member(i);
                    if (member->type == 'w') {
                        way2mpidx[member->ref].push_back(multipolygons.size());
                        num_ways++;
                    } else {
                        std::cerr << "warning: multipolygon/boundary relation " << relation->get_id() << " has a non-way member which was ignored" << std::endl;
                    }
                }

                Osmium::OSM::MultipolygonFromRelation *mp = new Osmium::OSM::MultipolygonFromRelation(r, is_boundary, num_ways, cb->multipolygon);
                multipolygons.push_back(mp);
            }

            // in pass 1
            void callback_after_relations() {
                std::cerr << "found " << multipolygons.size() << " multipolygons\n";
            }

            void handle_complete_multipolygon(Osmium::OSM::MultipolygonFromRelation *mp) {
                bool way_geometries_are_ok = true;
                std::cerr << "MP multi multi=" << mp->get_id() << " done\n";
                for (int i=0; i < mp->num_ways; i++) {
                    std::cerr << "  way=" << mp->member_ways[i].get_id() << "\n";

                    //geos::geom::Geometry *g = mp->member_ways[i].get_geometry();
                    geos::geom::Geometry *g = mp->member_ways[i].create_geos_geometry();
                    if (g) {
                        geos::io::WKTWriter wkt;
                        std::cerr << "  way geometry: " << wkt.write(g) << std::endl;
                    } else {
                        way_geometries_are_ok = false;
                    }
                }
                if (way_geometries_are_ok) {
                    //if (m->get_geometry()) {
                    if (mp->build_geometry()) {
                        geos::io::WKTWriter wkt;
                        std::cerr << "  mp geometry: " << wkt.write(mp->geometry) << std::endl;
                    } else {
                        std::cerr << "  geom build error: " << mp->geometry_error_message << "\n";
                    }
                    callback_multipolygon(mp);
                } else {
                    std::cerr << "  can´t build mp geometry because at least one way geometry is broken\n";
                }
                // free way copies
                // free relation
            }

            // in pass 2
            void callback_way(OSM::Way *way) {
                way2mpidx_t::iterator way2mpidx_iterator = way2mpidx.find(way->get_id());

                if (way2mpidx_iterator == way2mpidx.end()) { // not in any relation
                    if (way->is_closed()) { // way is closed, build simple multipolygon
                        Osmium::OSM::MultipolygonFromWay *mp = new Osmium::OSM::MultipolygonFromWay(way, way->create_geos_geometry());
                        std::cerr << "MP simple way_id=" << way->get_id() << "\n";
                        callback_multipolygon(mp);
                        delete mp;
                    }
                    return;
                }
                
                // is in at least one multipolygon relation

                std::vector<osm_object_id_t> v = way2mpidx_iterator->second;
                std::cerr << "MP way_id=" << way->get_id() << " is in " << v.size() << " multipolygons\n";

                // go through all the multipolygons this way is in
                for (unsigned int i=0; i < v.size(); i++) {
                    Osmium::OSM::MultipolygonFromRelation *mp = multipolygons[v[i]];
                    std::cerr << "MP multi way_id=" << way->get_id() << " is in relation_id=" << mp->get_id() << "\n";

                    // store copy of current way in multipolygon
                    mp->add_member_way(way);

                    if (mp->is_complete()) {
                        handle_complete_multipolygon(mp);
                        multipolygons[v[i]] = NULL;
                        delete mp;
                    }
                }
            }

            // in pass 2
            void callback_multipolygon(Osmium::OSM::Multipolygon *multipolygon) {
                cb->multipolygon(multipolygon);
            }

            void callback_final() {
#ifdef WITH_MULTIPOLYGON_PROFILING
                Osmium::OSM::MultipolygonFromRelation::print_timings();
#endif
            }

        }; // class Multipolygon

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_MULTIPOLYGON_HPP
