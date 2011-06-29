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

#include <osmium/osm/way.hpp>
#include <osmium/osm/area.hpp>

namespace Osmium {

    namespace Handler {

        class Multipolygon : public Base {

            /// a list of areas that need to be completed
            std::vector<Osmium::OSM::AreaFromRelation*> m_areas;

            // a map from way_id to a vector of indexes into the areas array
            // this is used to find in which multipolygon relations a way is
            typedef google::sparse_hash_map<osm_object_id_t, std::vector<osm_object_id_t> > way2areaidx_t;
            way2areaidx_t m_way2areaidx;

            bool m_attempt_repair;
            void (*m_callback_area)(Osmium::OSM::Area*);

            uint64_t m_count_ways_in_all_areas;

        public:

            Multipolygon(bool attempt_repair,
                         void (*callback_area)(Osmium::OSM::Area*))
                : Base(),
                  m_areas(),
                  m_way2areaidx(),
                  m_attempt_repair(attempt_repair),
                  m_callback_area(callback_area),
                  m_count_ways_in_all_areas(0) {
            }

            // in pass 1
            void callback_relation(Osmium::OSM::Relation* relation) {
                const char *type = relation->get_tag_by_key("type");

                // ignore relations without "type" tag
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
                        m_way2areaidx[member->ref].push_back(m_areas.size());
                        num_ways++;
                    } else {
                        std::cerr << "warning: multipolygon/boundary relation "
                                  << relation->get_id()
                                  << " has a non-way member which was ignored\n";
                    }
                }

                m_count_ways_in_all_areas += num_ways;

                Osmium::OSM::AreaFromRelation* area = new Osmium::OSM::AreaFromRelation(new Osmium::OSM::Relation(*relation), is_boundary, num_ways, m_callback_area, m_attempt_repair);
                m_areas.push_back(area);
            }

            // in pass 1
            void callback_after_relations() {
                if (Osmium::debug()) {
                    std::cerr << "found " << m_areas.size() << " areas (each needs "
                              << sizeof(Osmium::OSM::Area) << " bytes, thats together about "
                              << sizeof(Osmium::OSM::Area) * m_areas.size() / (1024 * 1024) << "MB)\n"
                              << "they used " << m_count_ways_in_all_areas << " ways (each will need "
                              << sizeof(Osmium::OSM::Way) << " bytes, thats in the worst case together about "
                              << sizeof(Osmium::OSM::Way) * m_count_ways_in_all_areas / (1024 * 1024) << "MB)\n";
                }
            }

            // in pass 2
            void callback_way(Osmium::OSM::Way* way) {
                way2areaidx_t::const_iterator way2areaidx_iterator(m_way2areaidx.find(way->get_id()));

                if (way2areaidx_iterator == m_way2areaidx.end()) { // not in any relation
                    if (way->is_closed() && way->node_count() >= 4) { // way is closed and has enough nodes, build simple multipolygon
#ifdef OSMIUM_WITH_GEOS
                        Osmium::OSM::AreaFromWay* area = new Osmium::OSM::AreaFromWay(way, way->create_geos_geometry());
#else
                        Osmium::OSM::AreaFromWay* area = new Osmium::OSM::AreaFromWay(way);
#endif // OSMIUM_WITH_GEOS
                        if (Osmium::debug()) {
                            std::cerr << "MP simple way_id=" << way->get_id() << "\n";
                        }
                        m_callback_area(area);
                        delete area;
                    }
                    return;
                }

                // is in at least one multipolygon relation

                std::vector<osm_object_id_t> v = way2areaidx_iterator->second;
                if (Osmium::debug()) std::cerr << "MP way_id=" << way->get_id() << " is in " << v.size() << " areas\n";

                // go through all the areas this way is in
                for (unsigned int i=0; i < v.size(); i++) {
                    Osmium::OSM::AreaFromRelation* area = m_areas[v[i]];
                    if (!area) {
                        throw std::runtime_error("Zero multipolygon. This should not happen. Reason can be a way appearing more than once in your input file.");
                    }
                    if (Osmium::debug()) {
                        std::cerr << "MP multi way_id=" << way->get_id() << " is in relation_id=" << area->get_id() << "\n";
                    }

                    // store copy of current way in multipolygon
                    area->add_member_way(way);

                    if (area->is_complete()) {
                        area->handle_complete_multipolygon();
                        m_areas[v[i]] = NULL;
                        delete area;
                    }
                }
            }

            // in pass 2
            void callback_after_ways() {
                m_way2areaidx.clear();
            }

            void callback_init() {
#ifdef OSMIUM_WITH_MULTIPOLYGON_PROFILING
                Osmium::OSM::AreaFromRelation::init_timings();
#endif // OSMIUM_WITH_MULTIPOLYGON_PROFILING
            }

            void callback_final() {
#ifdef OSMIUM_WITH_MULTIPOLYGON_PROFILING
                Osmium::OSM::AreaFromRelation::print_timings();
#endif // OSMIUM_WITH_MULTIPOLYGON_PROFILING
            }

        }; // class Multipolygon

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_MULTIPOLYGON_HPP
