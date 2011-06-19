#ifndef OSMIUM_HANDLER_MULTIPOLYGON_HPP
#define OSMIUM_HANDLER_MULTIPOLYGON_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <google/sparse_hash_map>

namespace Osmium {

    namespace Handler {

        class Multipolygon : public Base {

            /// a list of multipolygons that need to be completed
            std::vector<Osmium::OSM::MultipolygonFromRelation *> multipolygons;

            // a map from way_id to a vector of indexes into the multipolygons array
            // this is used to find in which multipolygon relations a way is
            typedef google::sparse_hash_map<osm_object_id_t, std::vector<osm_object_id_t> > way2mpidx_t;
            way2mpidx_t way2mpidx;

            bool attempt_repair;
            void (*callback_multipolygon_parent)(Osmium::OSM::Multipolygon *);

            uint64_t count_ways_in_all_multipolygons;

        public:

            Multipolygon(bool attempt_repair, void (*cb)(Osmium::OSM::Multipolygon *)) : Base(), attempt_repair(attempt_repair), callback_multipolygon_parent(cb) {
                count_ways_in_all_multipolygons = 0;
            }

            // in pass 1
            void callback_relation(OSM::Relation *relation) {
                const char *type = relation->get_tag_by_key("type");
                if (!type) {
                    return;
                }

                bool is_boundary;
                if (strcmp(type, "multipolygon") == 0) {
                    is_boundary = false;
                } else if (strcmp(type, "boundary") == 0) {
                    is_boundary = true;
                } else {
                    return;
                }

                int num_ways = 0;
                for (osm_sequence_id_t i=0; i < relation->members().size(); i++) {
                    const Osmium::OSM::RelationMember *member = relation->get_member(i);
                    if (member->type == 'w') {
                        way2mpidx[member->ref].push_back(multipolygons.size());
                        num_ways++;
                    } else {
                        std::cerr << "warning: multipolygon/boundary relation " << relation->get_id() << " has a non-way member which was ignored" << std::endl;
                    }
                }

                count_ways_in_all_multipolygons += num_ways;

                Osmium::OSM::MultipolygonFromRelation *mp = new Osmium::OSM::MultipolygonFromRelation(new Osmium::OSM::Relation(*relation), is_boundary, num_ways, callback_multipolygon_parent, attempt_repair);
                multipolygons.push_back(mp);
            }

            // in pass 1
            void callback_after_relations() {
                if (Osmium::global.debug) {
                    std::cerr << "found " << multipolygons.size() << " multipolygons (each needs " << sizeof(Osmium::OSM::Multipolygon) << " bytes, thats together about " << sizeof(Osmium::OSM::Multipolygon) * multipolygons.size() / (1024 * 1024) << "MB)" << std::endl;
                    std::cerr << "they used " << count_ways_in_all_multipolygons << " ways (each will need " << sizeof(Osmium::OSM::Way) << " bytes, thats in the worst case together about " << sizeof(Osmium::OSM::Way) * count_ways_in_all_multipolygons / (1024 * 1024) << "MB)" << std::endl;
                }
            }

            // in pass 2
            void callback_way(OSM::Way *way) {
                way2mpidx_t::const_iterator way2mpidx_iterator(way2mpidx.find(way->get_id()));

                if (way2mpidx_iterator == way2mpidx.end()) { // not in any relation
                    if (way->is_closed() && way->node_count() >= 4) { // way is closed and has enough nodes, build simple multipolygon
#ifdef OSMIUM_WITH_GEOS
                        Osmium::OSM::MultipolygonFromWay *mp = new Osmium::OSM::MultipolygonFromWay(way, way->create_geos_geometry());
#else
                        Osmium::OSM::MultipolygonFromWay *mp = new Osmium::OSM::MultipolygonFromWay(way);
#endif // OSMIUM_WITH_GEOS
                        if (Osmium::global.debug) std::cerr << "MP simple way_id=" << way->get_id() << "\n";
                        callback_multipolygon(mp);
                        delete mp;
                    }
                    return;
                }

                // is in at least one multipolygon relation

                std::vector<osm_object_id_t> v = way2mpidx_iterator->second;
                if (Osmium::global.debug) std::cerr << "MP way_id=" << way->get_id() << " is in " << v.size() << " multipolygons\n";

                // go through all the multipolygons this way is in
                for (unsigned int i=0; i < v.size(); i++) {
                    Osmium::OSM::MultipolygonFromRelation *mp = multipolygons[v[i]];
                    if (!mp) {
                        throw std::runtime_error("Zero multipolygon. This should not happen. Reason can be a way appearing more than once in your input file.");
                    }
                    if (Osmium::global.debug) std::cerr << "MP multi way_id=" << way->get_id() << " is in relation_id=" << mp->get_id() << "\n";

                    // store copy of current way in multipolygon
                    mp->add_member_way(way);

                    if (mp->is_complete()) {
                        mp->handle_complete_multipolygon();
                        multipolygons[v[i]] = NULL;
                        delete mp;
                    }
                }
            }

            // in pass 2
            void callback_after_ways() {
                way2mpidx.clear();
            }

            // in pass 2
            void callback_multipolygon(Osmium::OSM::Multipolygon *multipolygon) {
                callback_multipolygon_parent(multipolygon);
            }

            void callback_init() {
#ifdef OSMIUM_WITH_MULTIPOLYGON_PROFILING
                Osmium::OSM::MultipolygonFromRelation::init_timings();
#endif // OSMIUM_WITH_MULTIPOLYGON_PROFILING
            }

            void callback_final() {
#ifdef OSMIUM_WITH_MULTIPOLYGON_PROFILING
                Osmium::OSM::MultipolygonFromRelation::print_timings();
#endif // OSMIUM_WITH_MULTIPOLYGON_PROFILING
            }

        }; // class Multipolygon

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_MULTIPOLYGON_HPP
