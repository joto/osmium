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

namespace Osmium {

    namespace Relations {

        class MultiPolygonRelationInfo : public RelationInfo {

            bool m_is_boundary;

        public:

            MultiPolygonRelationInfo(const shared_ptr<Osmium::OSM::Relation const>& relation, bool is_boundary) :
                RelationInfo(relation),
                m_is_boundary(is_boundary) {
            }

        };

        template <class THandler>
        class MultiPolygonAssembler : public Assembler<MultiPolygonAssembler<THandler>, MultiPolygonRelationInfo, THandler> {

            typedef Assembler<MultiPolygonAssembler, MultiPolygonRelationInfo, THandler> AssemblerType;

            typedef typename AssemblerType::HandlerPass1                              HandlerPass1;
            typedef typename AssemblerType::template HandlerPass2<false, true, false> HandlerPass2;

            HandlerPass1 m_handler_pass1;
            HandlerPass2 m_handler_pass2;

            bool m_attempt_repair;

        public:

            MultiPolygonAssembler(THandler& handler, bool attempt_repair) :
                Assembler<MultiPolygonAssembler, MultiPolygonRelationInfo, THandler>(handler),
                m_handler_pass1(*this),
                m_handler_pass2(*this),
                m_attempt_repair(attempt_repair) {
            }

            HandlerPass1& handler_pass1() {
                return m_handler_pass1;
            }

            HandlerPass2& handler_pass2() {
                return m_handler_pass2;
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
                    return;
                }

                Assembler<MultiPolygonAssembler, MultiPolygonRelationInfo, THandler>::add_relation(MultiPolygonRelationInfo(relation, is_boundary));
            }

            bool keep_member(MultiPolygonRelationInfo& relation_info, const Osmium::OSM::RelationMember& member) {
                if (member.type() == 'w') {
                    return true;
                }
                std::cerr << "Ignored non-way member of multipolygon relation with id " << relation_info.relation()->id() << "\n";
                return false;
            }

            void way_not_in_any_relation(const shared_ptr<Osmium::OSM::Way const>& way) {
                if (way->is_closed()) {
                    // not in any relation, make simple polygon
//                m_handler.area(...);
                }
            }

            void complete_relation(Osmium::Relations::RelationInfo& relation_info) {
                std::cerr << "MP Rel completed: " << relation_info.relation()->id() << "\n";
                // create MP
//                m_handler.area(...);
            }

        };

    } // namespace Relations

} // namespace Osmium

#endif // OSMIUM_RELATIONS_MULTIPOLYGON_ASSEMBLER_HPP
