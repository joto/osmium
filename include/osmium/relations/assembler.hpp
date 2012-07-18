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

            RelationInfo(const shared_ptr<Osmium::OSM::Relation const>& relation) :
                m_relation(relation),
                m_members(),
                m_need_members(0) {
            }

            const shared_ptr<Osmium::OSM::Relation const>& relation() const {
                return m_relation;
            }

            void need_member() {
                ++m_need_members;
            }

            bool add_member(const shared_ptr<Osmium::OSM::Object const>& object) {
                m_members.push_back(object);
                return --m_need_members == 0;
            }

            std::vector< shared_ptr<Osmium::OSM::Object const> >& members() {
                return m_members;
            }

        };

        template <class TAssembler, class TRelationInfo, class THandler=Osmium::Handler::Base>
        class Assembler {

            typedef std::vector<TRelationInfo> relation_info_vector_t;

            typedef std::pair<osm_object_id_t, int> object_id_2_relation_info_num_t;
            typedef std::vector<object_id_2_relation_info_num_t> id2rel_vector_t;
            typedef std::pair<id2rel_vector_t::iterator, id2rel_vector_t::iterator> id2rel_range_t;

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

            void add_relation(TRelationInfo relation_info) {
                BOOST_FOREACH(const Osmium::OSM::RelationMember& member, relation_info.relation()->members()) {
                    bool add = static_cast<TAssembler*>(this)->keep_member(relation_info, member);
                    if (add) {
                        object_id_2_relation_info_num_t entry = std::make_pair(member.ref(), m_relations.size());
                        switch (member.type()) {
                            case 'n':
                                m_member_nodes.push_back(entry);
                                break;
                            case 'w':
                                m_member_ways.push_back(entry);
                                break;
                            case 'r':
                                m_member_relations.push_back(entry);
                                break;
                        }
                        relation_info.need_member();
                    }
                }
                m_relations.push_back(relation_info);
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

            }; // class HandlerPass1

            template<bool N, bool W, bool R>
            class HandlerPass2 {

                TAssembler& m_assembler;

                static bool compare_first(object_id_2_relation_info_num_t a, object_id_2_relation_info_num_t b) {
                    return a.first < b.first;
                }

            public:

                HandlerPass2(TAssembler& assembler) :
                    m_assembler(assembler) {
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
                        id2rel_range_t range = std::equal_range(m_assembler.m_member_nodes.begin(), m_assembler.m_member_nodes.end(), std::make_pair(node->id(), 0), compare_first);

                        if (range.first == range.second) {
                            m_assembler.node_not_in_any_relation(node);
                            return;
                        }

                        BOOST_FOREACH(object_id_2_relation_info_num_t x, range) {
                            TRelationInfo& relation_info = m_assembler.m_relations[x.second];
                            if (relation_info.add_member(node)) {
                                m_assembler.complete_relation(relation_info);
                            }
                        }
                    }
                    m_assembler.m_handler.node(node);
                }

                void after_nodes() {
                    if (N) {
                        m_assembler.m_member_nodes.clear();
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
                        id2rel_range_t range = std::equal_range(m_assembler.m_member_ways.begin(), m_assembler.m_member_ways.end(), std::make_pair(way->id(), 0), compare_first);

                        if (range.first == range.second) {
                            m_assembler.way_not_in_any_relation(way);
                            return;
                        }

                        BOOST_FOREACH(object_id_2_relation_info_num_t x, range) {
                            TRelationInfo& relation_info = m_assembler.m_relations[x.second];
                            if (relation_info.add_member(way)) {
                                m_assembler.complete_relation(relation_info);
                            }
                        }
                    }
                    m_assembler.m_handler.way(way);
                }

                void after_ways() {
                    if (W) {
                        m_assembler.m_member_ways.clear();
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
                        id2rel_range_t range = std::equal_range(m_assembler.m_member_relations.begin(), m_assembler.m_member_relations.end(), std::make_pair(relation->id(), 0), compare_first);

                        if (range.first == range.second) {
                            m_assembler.relation_not_in_any_relation(relation);
                            return;
                        }

                        BOOST_FOREACH(object_id_2_relation_info_num_t x, range) {
                            TRelationInfo& relation_info = m_assembler.m_relations[x.second];
                            if (relation_info.add_member(relation)) {
                                m_assembler.complete_relation(relation_info);
                            }
                        }
                    }
                    m_assembler.m_handler.relation(relation);
                }

                void after_relations() {
                    if (R) {
                        m_assembler.m_member_relations.clear();
                    }
                    m_assembler.m_handler.after_relations();
                }

                void final() const {
                    m_assembler.m_handler.final();
                }

            }; // class HandlerPass2

        private:

            const Osmium::Handler::Base m_base_handler;
            const THandler& m_handler;
            relation_info_vector_t m_relations;
            id2rel_vector_t m_member_nodes;
            id2rel_vector_t m_member_ways;
            id2rel_vector_t m_member_relations;

        }; // class Assembler

    } // namespace Relations

} // namespace Osmium

#endif // OSMIUM_RELATIONS_ASSEMBLER_HPP
