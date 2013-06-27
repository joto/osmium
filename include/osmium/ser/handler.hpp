#ifndef OSMIUM_SER_HANDLER_HPP
#define OSMIUM_SER_HANDLER_HPP

/*

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <boost/foreach.hpp>

#include <osmium/handler.hpp>
#include <osmium/ser/builder.hpp>
#include <osmium/ser/index.hpp>

namespace Osmium {

    namespace Ser {

        template<class TBufferManager, class TUpdateHandler, class TIndexNode, class TIndexWay, class TIndexRelation>
        class Handler : public Osmium::Handler::Base {

        public:

            Handler(TBufferManager& buffer_manager,
                    TUpdateHandler& update_handler,
                    bool update_mode,
                    TIndexNode& node_index,
                    TIndexWay& way_index,
                    TIndexRelation& relation_index) :
                Osmium::Handler::Base(),
                m_buffer_manager(buffer_manager),
                m_update_handler(update_handler),
                m_update_mode(update_mode),
                m_add_relation_member_objects(false),
                m_node_index(node_index),
                m_way_index(way_index),
                m_relation_index(relation_index) {
            }

            void add_relation_member_objects() {
                m_add_relation_member_objects = true;
            }

            void update_mode(bool mode=true) {
                m_update_mode = mode;
            }

            void init(Osmium::OSM::Meta&) const {
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                try {
                    write_node(node);
                } catch (Osmium::Ser::BufferIsFull&) {
                    m_buffer_manager.flush_buffer();
                    write_node(node);
                }
            }

            void after_nodes() {
                m_buffer_manager.flush_buffer();
                m_update_handler.after_nodes();
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& way) {
                try {
                    write_way(way);
                } catch (Osmium::Ser::BufferIsFull&) {
                    m_buffer_manager.flush_buffer();
                    write_way(way);
                }
            }

            void after_ways() {
                m_buffer_manager.flush_buffer();
                m_update_handler.after_ways();
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                try {
                    write_relation(relation);
                } catch (Osmium::Ser::BufferIsFull&) {
                    m_buffer_manager.flush_buffer();
                    write_relation(relation);
                }
            }

            void after_relations() {
                m_buffer_manager.flush_buffer();
                m_update_handler.after_relations();
            }

            void final() {
                m_update_handler.final();
            }

        private:

            TBufferManager& m_buffer_manager;
            TUpdateHandler& m_update_handler;

            bool m_update_mode;
            bool m_add_relation_member_objects;

            TIndexNode&     m_node_index;
            TIndexWay&      m_way_index;
            TIndexRelation& m_relation_index;

            Osmium::Ser::Buffer& buffer() {
                return m_buffer_manager.output_buffer();
            }

            template <class T, class TIndex>
            T* get_old_version(Osmium::Ser::Object& object, TIndex& index) {
                if (m_update_mode && object.version() > 1) {
                    try {
                        size_t offset = index.get(object.id());
                        return &m_buffer_manager.template get<T>(offset);
                    } catch (Osmium::Ser::Index::NotFound&) {
                    }
                }
                return NULL;
            }

            void write_node(const shared_ptr<Osmium::OSM::Node const>& node) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::Node> builder(buffer());

                Osmium::Ser::Node& sn = builder.object();
                sn.id(node->id());
                sn.version(node->version());
                sn.timestamp(node->timestamp());
                sn.uid(node->uid());
                sn.changeset(node->changeset());
                sn.position(node->position());

                builder.add_string(node->user());

                if (node->tags().size() > 0) {
                    builder.add_tags(node->tags());
                }

                assert(buffer().is_aligned());

                Osmium::Ser::Node* old_node = get_old_version<Osmium::Ser::Node>(sn, m_node_index);

                m_node_index.set(node->id(), m_buffer_manager.commit());

                m_update_handler.node(sn, old_node);
            }

            void write_way(const shared_ptr<Osmium::OSM::Way const>& way) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::Way> builder(buffer());

                Osmium::Ser::Way& sn = builder.object();
                sn.id(way->id());
                sn.version(way->version());
                sn.timestamp(way->timestamp());
                sn.uid(way->uid());
                sn.changeset(way->changeset());

                builder.add_string(way->user());
                builder.add_tags(way->tags());

                if (way->nodes()[0].position().defined()) {
                    builder.add_way_nodes_with_position(way->nodes());
                } else {
                    builder.add_way_nodes(way->nodes());
                }

                Osmium::Ser::Way* old_way = get_old_version<Osmium::Ser::Way>(sn, m_way_index);

                m_way_index.set(way->id(), m_buffer_manager.commit());

                m_update_handler.way(sn, old_way);
            }

            void write_relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::Relation> builder(buffer());

                Osmium::Ser::Relation& sn = builder.object();
                sn.id(relation->id());
                sn.version(relation->version());
                sn.timestamp(relation->timestamp());
                sn.uid(relation->uid());
                sn.changeset(relation->changeset());

                builder.add_string(relation->user());
                builder.add_tags(relation->tags());

                if (m_add_relation_member_objects) {
                    Osmium::Ser::ObjectBuilder<Osmium::Ser::RelationMemberList> rml_builder(buffer(), &builder);

                    BOOST_FOREACH(const Osmium::OSM::RelationMember& member, relation->members()) {
                        if (member.type() == 'n') {
                            try {
                                size_t offset = m_node_index.get(member.ref());
                                const Osmium::Ser::Node& node = m_buffer_manager.template get<Osmium::Ser::Node>(offset);
                                assert(member.ref() == node.id());
                                rml_builder.add_member(member.type(), member.ref(), member.role(), &node);
                            } catch (Osmium::Ser::Index::NotFound& err) {
                                rml_builder.add_member(member.type(), member.ref(), member.role());
                            }
                        } else if (member.type() == 'w') {
                            try {
                                size_t offset = m_way_index.get(member.ref());
                                const Osmium::Ser::Way& way = m_buffer_manager.template get<Osmium::Ser::Way>(offset);
                                assert(member.ref() == way.id());
                                rml_builder.add_member(member.type(), member.ref(), member.role(), &way);
                            } catch (Osmium::Ser::Index::NotFound& err) {
                                rml_builder.add_member(member.type(), member.ref(), member.role());
                            }
                        } else {
                            rml_builder.add_member(member.type(), member.ref(), member.role());
                        }
                    }
                    rml_builder.add_padding();
                } else {
                    builder.add_members(relation->members());
                }

                Osmium::Ser::Relation* old_relation = get_old_version<Osmium::Ser::Relation>(sn, m_relation_index);

                m_relation_index.set(relation->id(), m_buffer_manager.commit());

                m_update_handler.relation(sn, old_relation);
            }

        }; // class Handler

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_HANDLER_HPP
