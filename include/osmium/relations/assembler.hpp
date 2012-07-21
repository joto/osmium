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

    namespace Relations {

        class RelationInfo {

            shared_ptr<Osmium::OSM::Relation const> m_relation;
            std::vector< shared_ptr<Osmium::OSM::Object const> > m_members;
            int m_need_members;

        public:

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

            void need_member() {
                ++m_need_members;
            }

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

        struct has_all_members : public std::unary_function<RelationInfo, bool> {
            bool operator()(RelationInfo& relation_info) const {
                return relation_info.has_all_members();
            }
        };

        template <class TAssembler, class TRelationInfo, class THandler=Osmium::Handler::Base>
        class Assembler {

            typedef std::vector<TRelationInfo> relation_info_vector_t;

            struct MemberInfo {

                osm_object_id_t m_member_id;
                typename relation_info_vector_t::size_type m_relation_pos;
                osm_sequence_id_t m_member_pos;

                MemberInfo(osm_object_id_t member_id, typename relation_info_vector_t::size_type relation_pos=0, osm_sequence_id_t member_pos=0) :
                    m_member_id(member_id),
                    m_relation_pos(relation_pos),
                    m_member_pos(member_pos) {
                }

                friend operator<(const MemberInfo& a, const MemberInfo& b) {
                    return a.m_member_id < b.m_member_id;
                }

            };

            typedef std::vector<MemberInfo> member_info_vector_t;
            typedef std::pair<typename member_info_vector_t::iterator, typename member_info_vector_t::iterator> member_info_range_t;

        public:

            Assembler() :
                m_base_handler(),
                m_handler(m_base_handler),
                m_relations(),
                m_member_nodes(),
                m_member_ways(),
                m_member_relations() {
            }

            Assembler(THandler& handler) :
                m_base_handler(),
                m_handler(handler),
                m_relations(),
                m_member_nodes(),
                m_member_ways(),
                m_member_relations() {
            }

        protected:

            const relation_info_vector_t& relations() const {
                return m_relations;
            }

            // overwrite in child class
            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                add_relation(TRelationInfo(relation));
            }

            // overwrite in child class
            bool keep_member(Osmium::Relations::RelationInfo& /*relation_info*/, const Osmium::OSM::RelationMember& /*member*/) {
                return true;
            }

            // overwrite in child class
            void node_not_in_any_relation(const shared_ptr<Osmium::OSM::Node const>& /*node*/) {
            }

            // overwrite in child class
            void way_not_in_any_relation(const shared_ptr<Osmium::OSM::Way const>& /*way*/) {
            }

            // overwrite in child class
            void relation_not_in_any_relation(const shared_ptr<Osmium::OSM::Relation const>& /*relation*/) {
            }

            // overwrite in child class
            void all_members_available() {
            }

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
                return m_handler;
            }

        public:

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
                    m_assembler.m_handler.init(meta);
                }

                void before_nodes() {
                    if (N) {
                        std::sort(m_assembler.m_member_nodes.begin(), m_assembler.m_member_nodes.end());
                    }
                    m_assembler.m_handler.before_nodes();
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
                    m_assembler.m_handler.node(node);
                }

                void after_nodes() {
                    if (N) {
                        // clear all memory used by m_member_nodes
                        member_info_vector_t().swap(m_assembler.m_member_nodes);
                        if (--m_want_types == 0) {
                            m_assembler.all_members_available();
                        }
                    }
                    m_assembler.m_handler.after_nodes();
                }

                void before_ways() {
                    if (W) {
                        std::sort(m_assembler.m_member_ways.begin(), m_assembler.m_member_ways.end());
                    }
                    m_assembler.m_handler.before_ways();
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
                    m_assembler.m_handler.way(way);
                }

                void after_ways() {
                    if (W) {
                        // clear all memory used by m_member_ways
                        member_info_vector_t().swap(m_assembler.m_member_ways);
                        if (--m_want_types == 0) {
                            m_assembler.all_members_available();
                        }
                    }
                    m_assembler.m_handler.after_ways();
                }

                void before_relations() {
                    if (R) {
                        std::sort(m_assembler.m_member_relations.begin(), m_assembler.m_member_relations.end());
                    }
                    m_assembler.m_handler.before_relations();
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
                    m_assembler.m_handler.relation(relation);
                }

                void after_relations() {
                    if (R) {
                        // clear all memory used by m_member_relations
                        member_info_vector_t().swap(m_assembler.m_member_relations);
                        if (--m_want_types == 0) {
                            m_assembler.all_members_available();
                        }
                    }
                    m_assembler.m_handler.after_relations();
                }

                void final() const {
                    m_assembler.m_handler.final();
                }

            }; // class HandlerPass2

        private:

            Osmium::Handler::Base m_base_handler;
            THandler& m_handler;
            relation_info_vector_t m_relations;
            member_info_vector_t m_member_nodes;
            member_info_vector_t m_member_ways;
            member_info_vector_t m_member_relations;

        }; // class Assembler

    } // namespace Relations

} // namespace Osmium

#endif // OSMIUM_RELATIONS_ASSEMBLER_HPP
