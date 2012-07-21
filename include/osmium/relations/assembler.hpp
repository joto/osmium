#ifndef OSMIUM_RELATIONS_ASSEMBLER_HPP
#define OSMIUM_RELATIONS_ASSEMBLER_HPP

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

#include <vector>
#include <algorithm>
#include <assert.h>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>

#include <osmium/handler.hpp>

namespace Osmium {

    /**
     * @brief Namespace for code related to OSM relations
     */
    namespace Relations {

        /**
         * Helper class for the Assembler class.
         *
         * Stores a shared pointer to a relation plus the information needed to
         * add members to this relation.
         *
         * You can derive from this class in a child class of Assembler if you
         * need to store more information about a relation. See the MultiPolygonRelationInfo
         * class for an example.
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

            const std::vector< shared_ptr<Osmium::OSM::Object const> >& members() const {
                return m_members;
            }

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

        /**
         * The Assembler class assembles relations and their members. This is a generic
         * base class that can be used to assemble all kinds of relations. It has numerous
         * hooks you can implement in child classes to customize its behaviour.
         *
         * The assembler provides two handlers (HandlerPass1 and HandlerPass2) for a first
         * and second pass through an input file, respectively. In the first pass all
         * relations we are interested in are stored in TRelationInfo objects in the
         * m_relations vector. All members we are interested in are stored in MemberInfo
         * objects in the m_member_nodes, m_member_ways, and m_member_relations vectors.
         * The MemberInfo objects also store the information where the relations containing
         * those members are to be found.
         *
         * Later the m_member_(nodes|ways|relations) vectors are sorted according to the
         * member ids so that a binary search (with range_equal) can be used in the second
         * pass to find the relations for each node, way, or relation coming along. The
         * member objects are stored together with their relation and once a relation is
         * complete the complete_relation() method is called which you can overwrite in
         * a child class of Assembler.
         *
         * @tparam TAssembler Child class of this class.
         *
         * @tparam TRelationInfo RelationInfo or a child class of it.
         *
         * @tparam THandler Nested handler that is called at the end of each method in
         *         HandlerPass2. Defaults to Osmium::Handler::Base which does nothing.
         */
        template <class TAssembler, class TRelationInfo, class THandler=Osmium::Handler::Base>
        class Assembler {

            /// Vector of RelationInfo objects used to store relations we are interested in.
            typedef std::vector<TRelationInfo> relation_info_vector_t;

            /**
             *
             */
            struct MemberInfo {

                /**
                 * Object ID of this relation member. Can be a node, way, or relation ID.
                 * It depends on the vector in which this object is stored which kind of
                 * object is referenced here.
                 */
                osm_object_id_t m_member_id;

                /**
                 * Position of the relation this member is a part of in the
                 * m_relations vector.
                 */
                typename relation_info_vector_t::size_type m_relation_pos;

                /**
                 * Position of this member in the list of members of the
                 * relation this member if a part of.
                 */
                osm_sequence_id_t m_member_pos;

                /**
                 * Create new MemberInfo. The variant with zeros for relation_pos and
                 * member_pos is used to create dummy MemberInfo that can be compared
                 * to the MemberInfo in the vectors using the equal_range algorithm.
                 */
                MemberInfo(osm_object_id_t member_id, typename relation_info_vector_t::size_type relation_pos=0, osm_sequence_id_t member_pos=0) :
                    m_member_id(member_id),
                    m_relation_pos(relation_pos),
                    m_member_pos(member_pos) {
                }

                /**
                 * Compares two MemberInfo objects by only looking at the member id.
                 * Used to sort a vector of MemberInfo objects and to later find
                 * them using binary search.
                 */
                friend operator<(const MemberInfo& a, const MemberInfo& b) {
                    return a.m_member_id < b.m_member_id;
                }

            };

            typedef std::vector<MemberInfo> member_info_vector_t;

            /// This is the type used for results of the equal_range algorithm.
            typedef std::pair<typename member_info_vector_t::iterator, typename member_info_vector_t::iterator> member_info_range_t;

        public:

            /**
             * Create an Assembler without nested handler.
             */
            Assembler() :
                m_base_handler(),
                m_next_handler(m_base_handler),
                m_relations(),
                m_member_nodes(),
                m_member_ways(),
                m_member_relations() {
            }

            /**
             * Create an Assembler with nested handler.
             */
            Assembler(THandler& handler) :
                m_base_handler(),
                m_next_handler(handler),
                m_relations(),
                m_member_nodes(),
                m_member_ways(),
                m_member_relations() {
            }

        protected:

            const relation_info_vector_t& relations() const {
                return m_relations;
            }

            /**
             * This method is called from the first pass handler for every
             * relation in the input. It calls add_relation() to add this
             * relation to the list of relations we are interested in.
             *
             * Overwrite this method in a child class to only call add_relation
             * on the relations you are interested in, for instance depending
             * on the type tag. Storing relations takes a lot of memory, so
             * it makes sense to filter this as much as possible.
             */
            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                add_relation(TRelationInfo(relation));
            }

            /**
             * This method is called for every member of every relation that
             * is added with add_relation(). It should decide if the member
             * is interesting or not and return true or false to signal that.
             * Only interesting members are later added to the relation.
             *
             * Overwrite this method in a child class. In the MultiPolygonAssembler
             * this is for instance used to only keep members of type way and
             * ignore all others.
             */
            bool keep_member(Osmium::Relations::RelationInfo& /*relation_info*/, const Osmium::OSM::RelationMember& /*member*/) {
                return true;
            }

            /**
             * This method is called for all nodes that are not a member of
             * any relation.
             *
             * Overwrite this method in a child class if you are interested
             * in this.
             */
            void node_not_in_any_relation(const shared_ptr<Osmium::OSM::Node const>& /*node*/) {
            }

            /**
             * This method is called for all ways that are not a member of
             * any relation.
             *
             * Overwrite this method in a child class if you are interested
             * in this.
             */
            void way_not_in_any_relation(const shared_ptr<Osmium::OSM::Way const>& /*way*/) {
            }

            /**
             * This method is called for all relations that are not a member of
             * any relation.
             *
             * Overwrite this method in a child class if you are interested
             * in this.
             */
            void relation_not_in_any_relation(const shared_ptr<Osmium::OSM::Relation const>& /*relation*/) {
            }

            void all_members_available() {
            }

            /**
             * This removes all relations that have already been assembled
             * from the m_relations vector.
             */
            void clean_assembled_relations() {
                m_relations.erase(std::remove_if(m_relations.begin(), m_relations.end(), has_all_members()), m_relations.end());
            }

            void add_relation(TRelationInfo relation_info) {
                osm_sequence_id_t n=0;
                BOOST_FOREACH(const Osmium::OSM::RelationMember& member, relation_info.relation()->members()) {
                    bool add = static_cast<TAssembler*>(this)->keep_member(relation_info, member);
                    if (add) {
                        MemberInfo member_info(member.ref(), m_relations.size(), n);
                        switch (member.type()) {
                            case 'n':
                                m_member_nodes.push_back(member_info);
                                break;
                            case 'w':
                                m_member_ways.push_back(member_info);
                                break;
                            case 'r':
                                m_member_relations.push_back(member_info);
                                break;
                        }
                        relation_info.need_member();
                    }
                    n++;
                }
                m_relations.push_back(relation_info);
            }

            THandler& nested_handler() {
                return m_next_handler;
            }

        public:

            /**
             * This is the handler class for the first pass of the Assembler.
             */
            class HandlerPass1 : public Osmium::Handler::Base {

                TAssembler& m_assembler;

            public:

                HandlerPass1(TAssembler& assembler) :
                    Osmium::Handler::Base(),
                    m_assembler(assembler) {
                }

                void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                    m_assembler.relation(relation);
                }

                void after_relations() const {
                    throw Osmium::Handler::StopReading();
                }

            }; // class HandlerPass1

            /**
             * This is the handler class for the second pass of the Assembler.
             *
             * Instantiate this with the right template parameters in a child
             * class of Assembler.
             *
             * @tparam N Are we interested in member nodes?
             * @tparam W Are we interested in member ways?
             * @tparam R Are we interested in member relations?
             */
            template<bool N, bool W, bool R>
            class HandlerPass2 {

                TAssembler& m_assembler;

                int m_want_types;

            public:

                HandlerPass2(TAssembler& assembler) :
                    m_assembler(assembler),
                    m_want_types((N?1:0) + (W?1:0) + (R?1:0)) {
                }

                void init(Osmium::OSM::Meta& meta) const {
                    m_assembler.m_next_handler.init(meta);
                }

                void before_nodes() {
                    if (N) {
                        std::sort(m_assembler.m_member_nodes.begin(), m_assembler.m_member_nodes.end());
                    }
                    m_assembler.m_next_handler.before_nodes();
                }

                void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                    if (N) {
                        member_info_range_t range = std::equal_range(m_assembler.m_member_nodes.begin(), m_assembler.m_member_nodes.end(), MemberInfo(node->id()));

                        if (range.first == range.second) {
                            m_assembler.node_not_in_any_relation(node);
                            return;
                        }

                        BOOST_FOREACH(const MemberInfo& member_info, range) {
                            assert(member_info.m_member_id == node->id());
                            assert(member_info.m_relation_pos < m_assembler.m_relations.size());
                            TRelationInfo& relation_info = m_assembler.m_relations[member_info.m_relation_pos];
                            assert(member_info.m_member_pos < relation_info.relation()->members().size());
                            if (relation_info.add_member(node, member_info.m_member_pos)) {
                                m_assembler.complete_relation(relation_info);
                                m_assembler.m_relations[member_info.m_relation_pos] = TRelationInfo();
                            }
                        }
                    }
                    m_assembler.m_next_handler.node(node);
                }

                void after_nodes() {
                    if (N) {
                        // clear all memory used by m_member_nodes
                        member_info_vector_t().swap(m_assembler.m_member_nodes);
                        if (--m_want_types == 0) {
                            m_assembler.all_members_available();
                        }
                    }
                    m_assembler.m_next_handler.after_nodes();
                }

                void before_ways() {
                    if (W) {
                        std::sort(m_assembler.m_member_ways.begin(), m_assembler.m_member_ways.end());
                    }
                    m_assembler.m_next_handler.before_ways();
                }

                void way(const shared_ptr<Osmium::OSM::Way const>& way) {
                    if (W) {
                        member_info_range_t range = std::equal_range(m_assembler.m_member_ways.begin(), m_assembler.m_member_ways.end(), MemberInfo(way->id()));

                        if (range.first == range.second) {
                            m_assembler.way_not_in_any_relation(way);
                            return;
                        }

                        BOOST_FOREACH(const MemberInfo& member_info, range) {
                            assert(member_info.m_member_id == way->id());
                            assert(member_info.m_relation_pos < m_assembler.m_relations.size());
                            TRelationInfo& relation_info = m_assembler.m_relations[member_info.m_relation_pos];
                            assert(member_info.m_member_pos < relation_info.relation()->members().size());
                            if (relation_info.add_member(way, member_info.m_member_pos)) {
                                m_assembler.complete_relation(relation_info);
                                m_assembler.m_relations[member_info.m_relation_pos] = TRelationInfo();
                            }
                        }
                    }
                    m_assembler.m_next_handler.way(way);
                }

                void after_ways() {
                    if (W) {
                        // clear all memory used by m_member_ways
                        member_info_vector_t().swap(m_assembler.m_member_ways);
                        if (--m_want_types == 0) {
                            m_assembler.all_members_available();
                        }
                    }
                    m_assembler.m_next_handler.after_ways();
                }

                void before_relations() {
                    if (R) {
                        std::sort(m_assembler.m_member_relations.begin(), m_assembler.m_member_relations.end());
                    }
                    m_assembler.m_next_handler.before_relations();
                }

                void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                    if (R) {
                        member_info_range_t range = std::equal_range(m_assembler.m_member_relations.begin(), m_assembler.m_member_relations.end(), MemberInfo(relation->id()));

                        if (range.first == range.second) {
                            m_assembler.relation_not_in_any_relation(relation);
                            return;
                        }

                        BOOST_FOREACH(const MemberInfo& member_info, range) {
                            assert(member_info.m_member_id == relation->id());
                            assert(member_info.m_relation_pos < m_assembler.m_relations.size());
                            TRelationInfo& relation_info = m_assembler.m_relations[member_info.m_relation_pos];
                            assert(member_info.m_member_pos < relation_info.relation()->members().size());
                            if (relation_info.add_member(relation, member_info.m_member_pos)) {
                                m_assembler.complete_relation(relation_info);
                                m_assembler.m_relations[member_info.m_relation_pos] = TRelationInfo();
                            }
                        }
                    }
                    m_assembler.m_next_handler.relation(relation);
                }

                void after_relations() {
                    if (R) {
                        // clear all memory used by m_member_relations
                        member_info_vector_t().swap(m_assembler.m_member_relations);
                        if (--m_want_types == 0) {
                            m_assembler.all_members_available();
                        }
                    }
                    m_assembler.m_next_handler.after_relations();
                }

                void final() const {
                    m_assembler.m_next_handler.final();
                }

            }; // class HandlerPass2

        private:

            Osmium::Handler::Base m_base_handler;
            THandler& m_next_handler;
            relation_info_vector_t m_relations;
            member_info_vector_t m_member_nodes;
            member_info_vector_t m_member_ways;
            member_info_vector_t m_member_relations;

        }; // class Assembler

    } // namespace Relations

} // namespace Osmium

#endif // OSMIUM_RELATIONS_ASSEMBLER_HPP
