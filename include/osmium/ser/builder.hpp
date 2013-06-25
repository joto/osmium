#ifndef OSMIUM_SER_BUILDER_HPP
#define OSMIUM_SER_BUILDER_HPP

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

#include <osmium/ser/buffer.hpp>
#include <osmium/ser/item.hpp>

namespace Osmium {

    namespace Ser {

        class Builder {

        public:

            Builder(Buffer& buffer, Builder* parent, size_t size, ItemType itemtype) :
                m_buffer(buffer),
                m_parent(parent),
                m_item(reinterpret_cast<Osmium::Ser::TypedItem*>(m_buffer.get_space(size))) {
                assert(buffer.is_aligned());
                new (m_item) TypedItem(size, itemtype);
                if (m_parent) {
                    m_parent->add_size(size);
                }
            }

            void add_size(uint32_t size) {
                m_item->add_size(size);
                if (m_parent) {
                    m_parent->add_size(size);
                }
            }

            uint32_t size() const {
                return m_item->size();
            }

            void add_item(const Osmium::Ser::TypedItem* item) {
                memcpy(m_buffer.get_space(item->size()), item, item->size()); 
                add_size(item->size());
            }

            /**
             * Add padding if needed.
             *
             * Adds size to parent, but not to self!
             */
            void add_padding() {
                size_t padding = align_bytes - (size() % align_bytes);
                if (padding != align_bytes) {
                    m_buffer.get_space(padding);
                    if (m_parent) {
                        m_parent->add_size(padding);
                    }
                }
                assert(m_parent->size() % align_bytes == 0);
            }

            void add_string(const char* str) {
                size_t len = strlen(str) + 1;
                *m_buffer.get_space_for<size_t>() = len;
                m_buffer.append(str);
                add_size(sizeof(size_t) + len);

                size_t padding = align_bytes - (len % align_bytes);
                if (padding != align_bytes) {
                    m_buffer.get_space(padding);
                    add_size(padding);
                }
                assert(m_buffer.is_aligned());
            }

        protected:

            ~Builder() {
            }

            Buffer& m_buffer;
            Builder* m_parent;
            Osmium::Ser::TypedItem* m_item;

        }; // Builder

        template <class T>
        class ObjectBuilder : public Builder {

        public:

            ObjectBuilder(Buffer& buffer, Builder* parent=NULL) :
                Builder(buffer, parent, sizeof(T), item_traits<T>::itemtype) {
            }

            T& object() {
                return *static_cast<T*>(m_item);
            }

            void add_tag(const char* key, const char* value) {
                add_size(m_buffer.append(key) + m_buffer.append(value));
            }

            void add_tags(const Osmium::OSM::TagList& tags) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::TagList> builder(m_buffer, this);

                BOOST_FOREACH(const Osmium::OSM::Tag& tag, tags) {
                    builder.add_tag(tag.key(), tag.value());
                }

                builder.add_padding();
            }

            void add_way_node(const Osmium::OSM::WayNode& way_node) {
                new (m_buffer.get_space_for<Osmium::Ser::WayNode>()) Osmium::Ser::WayNode(way_node.ref());
                add_size(sizeof(Osmium::Ser::WayNode));
            }

            void add_way_node_with_position(const Osmium::OSM::WayNode& way_node) {
                new (m_buffer.get_space_for<Osmium::Ser::WayNodeWithPosition>()) Osmium::Ser::WayNodeWithPosition(way_node.ref(), way_node.position());
                add_size(sizeof(Osmium::Ser::WayNodeWithPosition));
            }

            void add_way_nodes(const Osmium::OSM::WayNodeList& nodes) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::WayNodeList> builder(m_buffer, this);

                BOOST_FOREACH(const Osmium::OSM::WayNode& way_node, nodes) {
                    builder.add_way_node(way_node);
                }

                builder.add_padding();
            }

            void add_way_nodes_with_position(const Osmium::OSM::WayNodeList& nodes) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::WayNodeWithPositionList> builder(m_buffer, this);

                BOOST_FOREACH(const Osmium::OSM::WayNode& way_node, nodes) {
                    builder.add_way_node_with_position(way_node);
                }

                builder.add_padding();
            }

            void add_member(char type, osm_object_id_t ref, const char* role, const Osmium::Ser::Object* full_member = NULL) {
                Osmium::Ser::RelationMember* member = m_buffer.get_space_for<Osmium::Ser::RelationMember>();
                new (member) Osmium::Ser::RelationMember(ref, type, full_member != NULL);
                add_size(sizeof(RelationMember));
                add_string(role);
                if (full_member) {
                    add_item(full_member);
                }
            }

            void add_members(const Osmium::OSM::RelationMemberList& members) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::RelationMemberList> builder(m_buffer, this);

                BOOST_FOREACH(const Osmium::OSM::RelationMember& member, members) {
                    builder.add_member(member.type(), member.ref(), member.role());
                }

                builder.add_padding();
            }

        }; // class ObjectBuilder

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_BUILDER_HPP
