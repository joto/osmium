#ifndef OSMIUM_HANDLER_MULTIPOLYGON_HPP
#define OSMIUM_HANDLER_MULTIPOLYGON_HPP

#include <google/sparse_hash_map>

#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>

namespace Osmium {

    namespace Handler {

        class Multipolygon : public Base {

            std::vector<Osmium::OSM::Multipolygon *> multipolygons;
            typedef google::sparse_hash_map<osm_object_id_t, std::vector<std::pair<osm_object_id_t, osm_sequence_id_t> > > way2rel_t;
            way2rel_t way2rel;

            struct callbacks *cb;

          public:

            Multipolygon(struct callbacks *cb) : cb(cb) {
            }

// TODO destructor that deletes multipolygons in vector
            // in pass 1
            void callback_before_relations() {
            }

            // in pass 2
            void callback_before_ways() {
                Osmium::OSM::Object::init(); // initialize geos lib
            }

            void add_to_way_lookup(int mp, OSM::Relation *relation, osm_sequence_id_t sequence_id) {
                osm_object_id_t member_id = relation->get_member(sequence_id)->ref;
                std::cerr << "mp relation=" << relation->get_id() << " seq_id=" << sequence_id << " is way=" << member_id << "\n";
                way2rel_t::iterator way2rel_iterator = way2rel.find(member_id);
                std::vector<std::pair<osm_object_id_t, osm_sequence_id_t> > *v;

                if (way2rel_iterator == way2rel.end()) {
                    std::cerr << "  new way2rel item\n";
                    v = new std::vector<std::pair<osm_object_id_t, osm_sequence_id_t> >;
                    std::pair<int, osm_sequence_id_t> *p = new std::pair<int, osm_sequence_id_t>(mp, sequence_id);
                    v->push_back(*p);
                    way2rel.insert(std::pair<osm_object_id_t, std::vector<std::pair<osm_object_id_t, osm_sequence_id_t> > >(member_id, *v));
                } else {
                    std::cerr << "  adding to way2rel item\n";
                    v = &(way2rel_iterator->second);
                    std::pair<int, osm_sequence_id_t> *p = new std::pair<int, osm_sequence_id_t>(mp, sequence_id);
                    v->push_back(*p);
                }

/*                std::pair<int, osm_sequence_id_t> *p = new std::pair<int, osm_sequence_id_t>(mp, sequence_id);
                v->push_back(*p);*/
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

                Osmium::OSM::Relation *r = new Osmium::OSM::Relation(relation);
                Osmium::OSM::Multipolygon *m = new Osmium::OSM::Multipolygon(r, is_boundary);
                multipolygons.push_back(m);

                for (int i=0; i < relation->member_count(); i++) {
                    if (relation->get_member(i)->type == 'w') {
                        add_to_way_lookup(multipolygons.size()-1, relation, i);
                        m->num_ways++;
                    }
                }

                m->missing_ways = m->num_ways;
            }

            // in pass 1
            void callback_after_relations() {
                std::cerr << "found " << multipolygons.size() << " multipolygons\n";
            }

            // in pass 2
            void callback_way(OSM::Way *way) {
                way2rel_t::iterator way2rel_iterator = way2rel.find(way->get_id());
                OSM::Multipolygon *mp;

                if (way2rel_iterator == way2rel.end()) { // not in any relation
                    if (way->is_closed()) { // way is closed, build simple multipolygon
                        mp = new OSM::Multipolygon(way);
                        std::cerr << "MP simple way=" << way->get_id() << "\n";
                        callback_multipolygon(mp);
                    }
                } else { // is in at least one multipolygon relation
                    std::vector<std::pair<osm_object_id_t, osm_sequence_id_t> > v = way2rel_iterator->second;

                    std::cerr << "MP multi way=" << way->get_id() << " is in " << v.size() << " multipolygons\n";
                    for (unsigned int i=0; i < v.size(); i++) {
                        std::pair<int, osm_sequence_id_t> *p = &v[i];
                        Osmium::OSM::Multipolygon *m = multipolygons[p->first];
                        std::cerr << "MP multi way=" << way->get_id() << " is in rel=" << m->get_id() << " at " << p->second << "\n";

                        OSM::Way *w = new OSM::Way(way);
                        m->member_ways.push_back(w);

                        m->missing_ways--;
                        if (m->missing_ways == 0) {
                            bool way_geometries_are_ok = true;
                            std::cerr << "MP multi multi=" << m->get_id() << " done\n";
                            for (int i=0; i < m->num_ways; i++) {
                                std::cerr << "  way=" << m->member_ways[i].get_id() << "\n";

                                try {
                                    geos::geom::Geometry *g = m->member_ways[i].get_geometry();
                                    geos::io::WKTWriter wkt;
                                    std::cerr << "  way geometry: " << wkt.write(g) << std::endl;
                                } catch (std::exception& e) {
                                    way_geometries_are_ok = false;
                                }
                            }
                            if (way_geometries_are_ok) {
                                if (m->build_geometry(m->relation)) {
                                    geos::io::WKTWriter wkt;
                                    std::cerr << "  mp geometry: " << wkt.write(m->geometry) << std::endl;
                                } else {
                                    std::cerr << "  geom build error: " << m->geometry_error_message << "\n";
                                }
                                callback_multipolygon(m);
                            } else {
                                std::cerr << "  can´t build mp geometry because at least one way geometry is broken\n";
                            }
                            // free way copies
                            // free relation
                        }
                    }

                }
            }

            // in pass 2
            void callback_multipolygon(Osmium::OSM::Multipolygon *multipolygon) {
                cb->multipolygon(multipolygon);
            }

            // in pass 2
            void callback_after_ways() {
            }

        }; // class Multipolygon

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_MULTIPOLYGON_HPP
