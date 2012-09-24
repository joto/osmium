#ifndef OSMIUM_OSM_RELATION_HPP
#define OSMIUM_OSM_RELATION_HPP

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

#include <boost/operators.hpp>

#include <osmium/osm/object.hpp>
#include <osmium/osm/relation_member_list.hpp>

namespace Osmium {

    namespace OSM {

        class Relation : public Object, boost::less_than_comparable<Relation> {

        public:

            Relation() :
                Object(),
                m_members() {
            }

            Relation(const Relation& relation) :
                Object(relation),
                m_members(relation.members()) {
            }

            const RelationMemberList& members() const {
                return m_members;
            }

            osm_object_type_t type() const {
                return RELATION;
            }

            void add_member(const char type, osm_object_id_t ref, const char* role) {
                m_members.add_member(type, ref, role);
            }

            const RelationMember* get_member(osm_sequence_id_t index) const {
                if (index < m_members.size()) {
                    return &m_members[index];
                }
                return NULL;
            }

        private:

            RelationMemberList m_members;

        }; // class Relation

        /**
         * Relations can be ordered by id and version.
         * Note that we use the absolute value of the id for a
         * better ordering of objects with negative ids.
         */
        inline bool operator<(const Relation& lhs, const Relation& rhs) {
            if (lhs.id() == rhs.id()) {
                return lhs.version() < rhs.version();
            } else {
                return abs(lhs.id()) < abs(rhs.id());
            }
        }

        /**
         * Ordering for shared_ptrs of Relations.
         */
        inline bool operator<(const shared_ptr<Relation const>& lhs, const shared_ptr<Relation const>& rhs) {
            return *lhs < *rhs;
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_HPP
