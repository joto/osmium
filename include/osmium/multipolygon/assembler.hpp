#ifndef OSMIUM_MULTIPOLYGON_ASSEMBLER_HPP
#define OSMIUM_MULTIPOLYGON_ASSEMBLER_HPP

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

#include <osmium/smart_ptr.hpp>
#include <osmium/debug.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/relations/relation_info.hpp>
#include <osmium/relations/assembler.hpp>
#include <osmium/multipolygon/builder.hpp>

namespace Osmium {

    /**
     * @brief Code related to the building of multipolygons from relations.
     */
    namespace MultiPolygon {

        /**
         * This class assembles MultiPolygons from relations tagged with
         * type=multipolygon or type=boundary.
         *
         * @tparam THandler Chained handler class.
         * @tparam TBuilder MultiPolygon Builder class.
         */
        template <class THandler, class TBuilder = Osmium::MultiPolygon::Builder>
        class Assembler : public Osmium::Relations::Assembler<Osmium::MultiPolygon::Assembler<THandler>, Osmium::Relations::RelationInfo, false, true, false, THandler>, public Osmium::WithDebug {

            typedef typename Osmium::Relations::Assembler<Osmium::MultiPolygon::Assembler<THandler>, Osmium::Relations::RelationInfo, false, true, false, THandler> AssemblerType;

            bool m_attempt_repair;

        public:

            Assembler(THandler& handler, bool attempt_repair) :
                AssemblerType(handler),
                m_attempt_repair(attempt_repair) {
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                const char* type = relation->tags().get_value_by_key("type");

                // ignore relations without "type" tag
                if (!type) {
                    return;
                }

                if ((!strcmp(type, "multipolygon")) || (!strcmp(type, "boundary"))) {
                    AssemblerType::add_relation(Osmium::Relations::RelationInfo(relation));
                }
            }

            /**
             * We are only interested in members of type way.
             *
             * Overwritten from the Assembler class.
             */
            bool keep_member(Osmium::Relations::RelationInfo& relation_info, const Osmium::OSM::RelationMember& member) {
                if (member.type() == 'w') {
                    return true;
                }
                if (debug && has_debug_level(1)) {
                    std::cout << "Ignored non-way member of multipolygon/boundary relation " << relation_info.relation()->id() << "\n";
                }
                return false;
            }

            void way_not_in_any_relation(const shared_ptr<Osmium::OSM::Way const>& way) {
                if (way->is_closed() && way->nodes().size() >= 4) { // way is closed and has enough nodes, build simple multipolygon
                    if (debug && has_debug_level(2)) {
                        std::cout << "MultiPolygon from way " << way->id() << "\n";
                    }
                    AssemblerType::nested_handler().area(make_shared<Osmium::OSM::Area>(*way));
                }
            }

            void complete_relation(Osmium::Relations::RelationInfo& relation_info) {
                if (debug && has_debug_level(2)) {
                    std::cout << "MultiPolygon from relation " << relation_info.relation()->id() << "\n";
                }

                TBuilder builder(relation_info, m_attempt_repair);

                BOOST_FOREACH(shared_ptr<Osmium::OSM::Area>& area, builder.build()) {
                    AssemblerType::nested_handler().area(area);
                }
            }

            void all_members_available() {
                if (debug && has_debug_level(1)) {
                    AssemblerType::clean_assembled_relations();
                    if (! AssemblerType::relations().empty()) {
                        std::cout << "Warning! Some member ways missing for these multipolygon relations:";
                        BOOST_FOREACH(const Osmium::Relations::RelationInfo& relation_info, AssemblerType::relations()) {
                            std::cout << " " << relation_info.relation()->id();
                        }
                        std::cout << "\n";
                    }
                }
            }

        }; // class Assembler

    } // namespace MultiPolygon

} // namespace Osmium

#endif // OSMIUM_MULTIPOLYGON_ASSEMBLER_HPP
