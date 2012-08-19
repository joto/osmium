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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>

#include <osmium/handler.hpp>
#include <osmium/relations/relation_info.hpp>

namespace Osmium {

    /**
     * @brief Namespace for code related to OSM relations
     */
    namespace Relations {

        /**
         * Helper class for the Assembler class.
         *
         * Stores an object ID and information where the object should be
         * stored.
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
            unsigned int m_relation_pos;

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
            MemberInfo(osm_object_id_t member_id, int relation_pos=0, osm_sequence_id_t member_pos=0) :
                m_member_id(member_id),
                m_relation_pos(relation_pos),
                m_member_pos(member_pos) {
            }

        };

        /**
         * Compares two MemberInfo objects by only looking at the member id.
         * Used to sort a vector of MemberInfo objects and to later find
         * them using binary search.
         */
        bool operator<(const MemberInfo& a, const MemberInfo& b) {
            return a.m_member_id < b.m_member_id;
        }

        typedef std::vector<MemberInfo> member_info_vector_t;

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
         * member ids so that a binary search (with equal_range) can be used in the second
         * pass to find the relations for each node, way, or relation coming along. The
         * member objects are stored together with their relation and once a relation is
         * complete the complete_relation() method is called which you can overwrite in
         * a child class of Assembler.
         *
         * @tparam TAssembler Child class of this class.
         *
         * @tparam TRelationInfo RelationInfo or a child class of it.
         *
         * @tparam N Are we interested in member nodes?
         *
         * @tparam W Are we interested in member ways?
         *
         * @tparam R Are we interested in member relations?
         *
         * @tparam THandler Nested handler that is called at the end of each method in
         *         HandlerPass2. Defaults to Osmium::Handler::Base which does nothing.
         */
        template <class TAssembler, class TRelationInfo, bool N, bool W, bool R, class THandler=Osmium::Handler::Base>
        class Assembler {

            /// Vector of RelationInfo objects used to store relations we are interested in.
            typedef std::vector<TRelationInfo> relation_info_vector_t;

            /// This is the type used for results of the equal_range algorithm.
            typedef std::pair<typename member_info_vector_t::iterator, typename member_info_vector_t::iterator> member_info_range_t;

        public:

            /**
             * Create an Assembler without nested handler.
             */
            Assembler() :
                m_base_handler(),
                m_next_handler(m_base_handler),
                m_handler_pass1(*static_cast<TAssembler*>(this)),
                m_handler_pass2(*static_cast<TAssembler*>(this)),
                m_relations(),
                m_member_infos() {
            }

            /**
             * Create an Assembler with nested handler.
             */
            Assembler(THandler& handler) :
                m_base_handler(),
                m_next_handler(handler),
                m_handler_pass1(*static_cast<TAssembler*>(this)),
                m_handler_pass2(*static_cast<TAssembler*>(this)),
                m_relations(),
                m_member_infos() {
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

            /**
             * This method is called from the handler when all objects that are
             * wanted as relation members are available.
             *
             * Overwrite this method in a child class if you are interested
             * in this.
             *
             * Note that even after this call members might be missing if they
             * were not in the input file!
             */
            void all_members_available() {
            }

            /**
             * This removes all relations that have already been assembled
             * from the m_relations vector.
             */
            void clean_assembled_relations() {
                m_relations.erase(std::remove_if(m_relations.begin(), m_relations.end(), has_all_members()), m_relations.end());
            }

            /**
             * Tell the Assembler that you are interested in this relation
             * and want it kept until all members have been assembled and
             * it is handed back to you.
             */
            void add_relation(TRelationInfo relation_info) {
                osm_sequence_id_t n=0;
                BOOST_FOREACH(const Osmium::OSM::RelationMember& member, relation_info.relation()->members()) {
                    bool add = static_cast<TAssembler*>(this)->keep_member(relation_info, member);
                    if (add) {
                        MemberInfo member_info(member.ref(), m_relations.size(), n);
                        switch (member.type()) {
                            case 'n':
                                m_member_infos[NODE].push_back(member_info);
                                break;
                            case 'w':
                                m_member_infos[WAY].push_back(member_info);
                                break;
                            case 'r':
                                m_member_infos[RELATION].push_back(member_info);
                                break;
                        }
                        relation_info.need_member();
                    }
                    n++;
                }
                m_relations.push_back(relation_info);
            }

            /**
             * Return a reference to the nested handler that is called
             * in the second pass after the Assembler's own handler.
             */
            THandler& nested_handler() {
                return m_next_handler;
            }

            /**
             * Sort the vectors with the member infos so that we can do binary
             * search on them.
             */
            void sort_member_infos() {
                std::sort(m_member_infos[NODE].begin(),     m_member_infos[NODE].end());
                std::sort(m_member_infos[WAY].begin(),      m_member_infos[WAY].end());
                std::sort(m_member_infos[RELATION].begin(), m_member_infos[RELATION].end());
            }

        public:

            uint64_t used_memory() const {
                uint64_t nmembers = m_member_infos[NODE].size() + m_member_infos[WAY].size() + m_member_infos[RELATION].size();
                uint64_t relations = m_relations.size() * (sizeof(TRelationInfo) + sizeof(Osmium::OSM::Relation)) + nmembers * sizeof(Osmium::OSM::RelationMember); // plus tags
                uint64_t members = nmembers * sizeof(MemberInfo);

                std::cout << "nR  = m_relations.size()             = " << m_relations.size() << "\n";
                std::cout << "nMN = m_member_info[NODE].size()     = " << m_member_infos[NODE].size() << "\n";
                std::cout << "nMW = m_member_info[WAY].size()      = " << m_member_infos[WAY].size() << "\n";
                std::cout << "nMR = m_member_info[RELATION].size() = " << m_member_infos[RELATION].size() << "\n";
                std::cout << "nM  = m_member_info[*].size()        = " << nmembers << "\n";

                std::cout << "sRI = sizeof(TRelationInfo)  = " << sizeof(TRelationInfo) << "\n";
                std::cout << "sR  = sizeof(Relation)       = " << sizeof(Osmium::OSM::Relation) << "\n";
                std::cout << "sRM = sizeof(RelationMember) = " << sizeof(Osmium::OSM::RelationMember) << "\n";
                std::cout << "sMI = sizeof(MemberInfo)     = " << sizeof(MemberInfo) << "\n";

                std::cout << "nR * (sRI + sR) = " << m_relations.size() * (sizeof(TRelationInfo) + sizeof(Osmium::OSM::Relation)) << "\n";
                std::cout << "nM * sRM        = " << nmembers * sizeof(Osmium::OSM::RelationMember) << "\n";
                std::cout << "nM * sMI        = " << nmembers * sizeof(MemberInfo) << "\n";

                return relations + members;
            }

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
                    m_assembler.sort_member_infos();
                    throw Osmium::Handler::StopReading();
                }

            }; // class HandlerPass1

            /**
             * This is the handler class for the second pass of the Assembler.
             */
            class HandlerPass2 {

                TAssembler& m_assembler;

                /**
                 * This variable is initialized with the number of different
                 * kinds of OSM objects we are interested in. If we only need
                 * way members (for instance for the multipolygon assembler)
                 * it is intialized with 1 for instance. If node and way
                 * members are needed, it is initialized with 2.
                 *
                 * In the after_* methods of this handler, it is decremented
                 * and once it reaches 0, we know we have all members available
                 * that we are ever going to get.
                 */
                int m_want_types;

                /**
                 * Find this object in the member vectors and add it to all
                 * relations that need it.
                 *
                 * @returns true if the member was added to at least one
                 *          relation and false otherwise
                 */
                bool find_and_add_object(const shared_ptr<Osmium::OSM::Object const>& object) const {
                    member_info_vector_t& miv = m_assembler.m_member_infos[object->type()];
                    const member_info_range_t range = std::equal_range(miv.begin(), miv.end(), MemberInfo(object->id()));

                    if (range.first == range.second) {
                        // nothing found
                        return false;
                    }

                    BOOST_FOREACH(const MemberInfo& member_info, range) {
                        assert(member_info.m_member_id == object->id());
                        assert(member_info.m_relation_pos < m_assembler.m_relations.size());
                        TRelationInfo& relation_info = m_assembler.m_relations[member_info.m_relation_pos];
                        assert(member_info.m_member_pos < relation_info.relation()->members().size());
                        if (relation_info.add_member(object, member_info.m_member_pos)) {
                            m_assembler.complete_relation(relation_info);
                            m_assembler.m_relations[member_info.m_relation_pos] = TRelationInfo();
                        }
                    }

                    return true;
                }

                /**
                 * This method is called from the after_* methods. It reclaimes
                 * memory that's not needed any more and calls
                 * all_members_available() if they are.
                 */
                void after(osm_object_type_t type) {
                    // clear all memory used by m_member_info of this type
                    member_info_vector_t().swap(m_assembler.m_member_infos[type]);
                    if (--m_want_types == 0) {
                        m_assembler.all_members_available();
                    }
                }

            public:

                HandlerPass2(TAssembler& assembler) :
                    m_assembler(assembler),
                    m_want_types((N?1:0) + (W?1:0) + (R?1:0)) {
                }

                void init(Osmium::OSM::Meta& meta) const {
                    m_assembler.m_next_handler.init(meta);
                }

                void before_nodes() const {
                    m_assembler.m_next_handler.before_nodes();
                }

                void node(const shared_ptr<Osmium::OSM::Node const>& node) const {
                    if (N) {
                        if (! find_and_add_object(node)) {
                            m_assembler.node_not_in_any_relation(node);
                        }
                    }
                    m_assembler.m_next_handler.node(node);
                }

                void after_nodes() {
                    if (N) {
                        after(NODE);
                    }
                    m_assembler.m_next_handler.after_nodes();
                }

                void before_ways() const {
                    m_assembler.m_next_handler.before_ways();
                }

                void way(const shared_ptr<Osmium::OSM::Way const>& way) const {
                    if (W) {
                        if (! find_and_add_object(way)) {
                            m_assembler.way_not_in_any_relation(way);
                        }
                    }
                    m_assembler.m_next_handler.way(way);
                }

                void after_ways() {
                    if (W) {
                        after(WAY);
                    }
                    m_assembler.m_next_handler.after_ways();
                }

                void before_relations() const {
                    m_assembler.m_next_handler.before_relations();
                }

                void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) const {
                    if (R) {
                        if (! find_and_add_object(relation)) {
                            m_assembler.relation_not_in_any_relation(relation);
                        }
                    }
                    m_assembler.m_next_handler.relation(relation);
                }

                void after_relations() {
                    if (R) {
                        after(RELATION);
                    }
                    m_assembler.m_next_handler.after_relations();
                }

                void final() const {
                    m_assembler.m_next_handler.final();
                }

            }; // class HandlerPass2

            /**
             * Return reference to first pass handler.
             */
            HandlerPass1& handler_pass1() {
                return m_handler_pass1;
            }

            /**
             * Return reference to second pass handler.
             */
            HandlerPass2& handler_pass2() {
                return m_handler_pass2;
            }

        private:

            /**
             * This base handler is used as default if no chained handler was
             * given to the Assembler.
             */
            Osmium::Handler::Base m_base_handler;

            /// Reference to chained handler
            THandler& m_next_handler;

            HandlerPass1 m_handler_pass1;
            HandlerPass2 m_handler_pass2;

            /// Vector with all relations we are interested in
            relation_info_vector_t m_relations;

            /**
             * One vector each for nodes, ways, and relations containing all
             * mappings from member ids to their relations.
             */
            member_info_vector_t m_member_infos[3];

        }; // class Assembler

    } // namespace Relations

} // namespace Osmium

#endif // OSMIUM_RELATIONS_ASSEMBLER_HPP
