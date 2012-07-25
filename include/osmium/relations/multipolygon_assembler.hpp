#ifndef OSMIUM_RELATIONS_MULTIPOLYGON_ASSEMBLER_HPP
#define OSMIUM_RELATIONS_MULTIPOLYGON_ASSEMBLER_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#define OSMIUM_COMPILE_WITH_CFLAGS_GEOS `geos-config --cflags`
#define OSMIUM_LINK_WITH_LIBS_GEOS `geos-config --libs`

#include <osmium/relations/assembler.hpp>
#include <osmium/debug.hpp>
#include <osmium/osm/area.hpp>

#include <osmium/relations/multipolygon_relationinfo.hpp>
#include <osmium/relations/area.hpp>

namespace Osmium {

    namespace Relations {

        /**
         * This class assembles MultiPolygons from relations tagged with
         * type=multipolygon or type=boundary.
         */
        template <class THandler>
        class MultiPolygonAssembler : public Assembler<MultiPolygonAssembler<THandler>, MultiPolygonRelationInfo, false, true, false, THandler>, public WithDebugLevel {

            typedef Assembler<MultiPolygonAssembler, MultiPolygonRelationInfo, false, true, false, THandler> AssemblerType;

            bool m_attempt_repair;

        public:

            MultiPolygonAssembler(THandler& handler, bool attempt_repair) :
                AssemblerType(handler),
                m_attempt_repair(attempt_repair) {
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                const char* type = relation->tags().get_tag_by_key("type");

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
                    // ignore relations that are not of type "multipolygon" or "boundary"
                    return;
                }

                AssemblerType::add_relation(MultiPolygonRelationInfo(relation, is_boundary));
            }

            /**
             * We are only interested in members of type way.
             *
             * Overwritten from the Assembler class.
             */
            bool keep_member(MultiPolygonRelationInfo& relation_info, const Osmium::OSM::RelationMember& member) {
                if (member.type() == 'w') {
                    return true;
                }
                OSMIUM_DEBUG(1, "Ignored non-way member of multipolygon/boundary relation " << relation_info.relation()->id() << "\n");
                return false;
            }

            void way_not_in_any_relation(const shared_ptr<Osmium::OSM::Way const>& way) {
                if (way->is_closed() && way->node_count() >= 4) { // way is closed and has enough nodes, build simple multipolygon
                    shared_ptr<Osmium::OSM::Area> area(make_shared<Osmium::OSM::Area>(*way));

                    OSMIUM_DEBUG(2, "MultiPolygon from way " << way->id() << "\n");

                    AssemblerType::nested_handler().area(area);
                }
            }

            void complete_relation(MultiPolygonRelationInfo& relation_info) {
                OSMIUM_DEBUG(2, "MultiPolygon from relation " << relation_info.relation()->id() << "\n");

                std::vector<shared_ptr<Osmium::OSM::Area> > areas;

                Osmium::Relations::AreaBuilder builder(relation_info, areas, m_attempt_repair);

                BOOST_FOREACH(const shared_ptr<Osmium::OSM::Object const>& way, relation_info.members()) {
                    if (way) {
                        OSMIUM_DEBUG(2, "  with member way " << way->id() << "\n");
                        builder.add_member_way(static_cast<Osmium::OSM::Way*>(const_cast<Osmium::OSM::Object*>(way.get()))); // XXX argh
                    }
                }

                builder.handle_complete_multipolygon();

                BOOST_FOREACH(shared_ptr<Osmium::OSM::Area>& area, areas) {
                    AssemblerType::nested_handler().area(area);
                }
            }

#ifdef OSMIUM_WITH_DEBUG
            void all_members_available() {
                if (debug_level() == 0) {
                    return;
                }

                AssemblerType::clean_assembled_relations();
                if (! AssemblerType::relations().empty()) {
                    std::cout << "Warning! Some member ways missing for these multipolygon relations:";
                    BOOST_FOREACH(const MultiPolygonRelationInfo& relation_info, AssemblerType::relations()) {
                        std::cout << " " << relation_info.relation()->id();
                    }
                    std::cout << "\n";
                }
            }
#endif // OSMIUM_WITH_DEBUG

        };

    } // namespace Relations

} // namespace Osmium

#endif // OSMIUM_RELATIONS_MULTIPOLYGON_ASSEMBLER_HPP
