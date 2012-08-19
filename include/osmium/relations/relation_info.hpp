#ifndef OSMIUM_RELATIONS_RELATION_INFO_HPP
#define OSMIUM_RELATIONS_RELATION_INFO_HPP

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

#include <functional>
#include <vector>

#include <osmium/smart_ptr.hpp>
#include <osmium/osm/relation.hpp>

namespace Osmium {

    namespace Relations {

        /**
         * Helper class for the Assembler class.
         *
         * Stores a shared pointer to a relation plus the information needed to
         * add members to this relation.
         *
         * You can derive from this class in a child class of Assembler if you
         * need to store more information about a relation. See the
         * MultiPolygonRelationInfo class for an example.
         */
        class RelationInfo {

            /// The relation we are assembling.
            shared_ptr<Osmium::OSM::Relation const> m_relation;

            /// Vector for relation members. Is initialized with the right size and empty objects.
            std::vector< shared_ptr<Osmium::OSM::Object const> > m_members;

            /**
             * The number of members still needed before the relation is complete.
             * This will be set to the number of members we are interested in and
             * then count down for every member we find. When it is 0, the relation
             * is complete.
             */
            int m_need_members;

        public:

            /**
             * Initialize an empty RelationInfo. This is needed to zero out relations that
             * have been completed and thus are removed from the m_relations vector.
             */
            RelationInfo() :
                m_relation(),
                m_members(),
                m_need_members(0) {
            }

            RelationInfo(const shared_ptr<Osmium::OSM::Relation const>& relation) :
                m_relation(relation),
                m_members(relation->members().size()),
                m_need_members(0) {
            }

            const shared_ptr<Osmium::OSM::Relation const>& relation() const {
                return m_relation;
            }

            int need_members() const {
                return m_need_members;
            }

            /**
             * There is one more member we need.
             */
            void need_member() {
                ++m_need_members;
            }

            /**
             * Add new member. This stores a shared pointer to the member object
             * in the RelationInfo. It also decrements the members needed counter.
             *
             * @return true if relation is complete, false otherwise
             */
            bool add_member(const shared_ptr<Osmium::OSM::Object const>& object, osm_sequence_id_t n) {
                assert(m_need_members > 0);
                assert(n < m_relation->members().size());
                m_members[n] = object;
                return --m_need_members == 0;
            }

            /**
             * Get a vector reference with shared pointers to all member objects.
             * Note that the pointers can be empty if a member object is of a type
             * we have not requested from the assembler (or if it was not in the
             * input).
             */
            const std::vector< shared_ptr<Osmium::OSM::Object const> >& members() const {
                return m_members;
            }

            /**
             * Returns true if all members for this relation are available.
             */
            bool has_all_members() const {
                return m_need_members == 0;
            }
        };

        /**
         * Function object to check if a relation is complete.
         *
         * @return true if this relation is complete, false otherwise.
         */
        struct has_all_members : public std::unary_function<RelationInfo, bool> {
            bool operator()(RelationInfo& relation_info) const {
                return relation_info.has_all_members();
            }
        };

    } // namespace Relations

} // namespace Osmium

#endif // OSMIUM_RELATIONS_RELATION_INFO_HPP
