#ifndef OSMIUM_SER_DESERIALIZER_HPP
#define OSMIUM_SER_DESERIALIZER_HPP

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

#include <stdio.h>
#include <utility>
#include <iostream>

#include <boost/foreach.hpp>

#include <osmium/smart_ptr.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/tag_list.hpp>
#include <osmium/handler.hpp>

#include <osmium/ser/buffer.hpp>
#include <osmium/ser/item.hpp>

namespace Osmium {

    /* namespace for code related to serializing osm objects */
    namespace Ser {

        namespace {

            template <class TObject>
            void parse_tag_list(const Osmium::Ser::TagList& tags, shared_ptr<TObject>& object) {
                for (Osmium::Ser::TagList::iterator it = tags.begin(); it != tags.end(); ++it) {
                    object->tags().add(it->key(), it->value());
                }
            }

            template <class TObject>
            void parse_way_node_list(const Osmium::Ser::WayNodeList&, shared_ptr<TObject>&) {
            }

            template <>
            void parse_way_node_list<Osmium::OSM::Way>(const Osmium::Ser::WayNodeList& nodes, shared_ptr<Osmium::OSM::Way>& way) {
                for (Osmium::Ser::WayNodeList::iterator it = nodes.begin(); it != nodes.end(); ++it) {
                    way->nodes().add(it->id());
                }
            }

            template <class TObject>
            void parse_way_node_list_with_position(const Osmium::Ser::WayNodeWithPositionList&, shared_ptr<TObject>&) {
            }

            template <>
            void parse_way_node_list_with_position<Osmium::OSM::Way>(const Osmium::Ser::WayNodeWithPositionList& nodes, shared_ptr<Osmium::OSM::Way>& way) {
                for (Osmium::Ser::WayNodeWithPositionList::iterator it = nodes.begin(); it != nodes.end(); ++it) {
                    way->nodes().add(Osmium::OSM::WayNode(it->id(), it->position()));
                }
            }

            template <class TObject>
            void parse_relation_member_list(const Osmium::Ser::RelationMemberList&, shared_ptr<TObject>&) {
            }

            template <>
            void parse_relation_member_list<Osmium::OSM::Relation>(const Osmium::Ser::RelationMemberList& members, shared_ptr<Osmium::OSM::Relation>& relation) {
                for (Osmium::Ser::RelationMemberList::iterator it = members.begin(); it != members.end(); ++it) {
                    relation->add_member(it->type().as_char(), it->ref(), it->role());
                    if (it->full_member()) {
                        const Osmium::Ser::Object& object = it->get_object();
                        std::cout << "  id=" << object.id() << "\n";
                    }
                }
            }

        } // anon namespace

        template <class THandler>
        class Deserializer {

        public:

            Deserializer(const Osmium::Ser::Buffer& data, THandler& handler) : m_data(data), m_offset(0), m_handler(handler) {
            }

            ~Deserializer() {
            }

            void dump() {
                while (m_offset < m_data.committed()) {
                    const Osmium::Ser::TypedItem& item = m_data.get<Osmium::Ser::TypedItem>(m_offset);
                    std::cout << "object type=" << item.type() << " size=" << item.size() << "\n";
                    m_offset += item.padded_size();
                }
            }

            template <class TItem, class TObject>
            void parse_subitems(const TItem& item, shared_ptr<TObject>& object) {
                for (Osmium::Ser::Object::iterator it = item.begin(); it != item.end(); ++it) {
                    switch (it->type().t()) {
                        case Osmium::Ser::ItemType::itemtype_tag_list:
                            parse_tag_list(reinterpret_cast<const Osmium::Ser::TagList&>(*it), object);
                            break;
                        case Osmium::Ser::ItemType::itemtype_way_node_list:
                            parse_way_node_list(reinterpret_cast<const Osmium::Ser::WayNodeList&>(*it), object);
                            break;
                        case Osmium::Ser::ItemType::itemtype_way_node_with_position_list:
                            parse_way_node_list_with_position(reinterpret_cast<const Osmium::Ser::WayNodeWithPositionList&>(*it), object);
                            break;
                        case Osmium::Ser::ItemType::itemtype_relation_member_list:
                        case Osmium::Ser::ItemType::itemtype_relation_member_list_with_full_members:
                            parse_relation_member_list(reinterpret_cast<const Osmium::Ser::RelationMemberList&>(*it), object);
                            break;
                        default:
                            throw std::runtime_error("parse error");
                    }
                }
            }

            void parse_node(const Osmium::Ser::Node& node_item) {
                shared_ptr<Osmium::OSM::Node> node = make_shared<Osmium::OSM::Node>();
                node->id(node_item.id());
                node->version(node_item.version());
                node->uid(node_item.uid());
                node->changeset(node_item.changeset());
                node->timestamp(node_item.timestamp());
                node->position(node_item.position());
                node->user(node_item.user());

                parse_subitems(node_item, node);

                m_handler.node(node);
            }

            void parse_way(const Osmium::Ser::Way& way_item) {
                shared_ptr<Osmium::OSM::Way> way = make_shared<Osmium::OSM::Way>();
                way->id(way_item.id());
                way->version(way_item.version());
                way->uid(way_item.uid());
                way->changeset(way_item.changeset());
                way->timestamp(way_item.timestamp());
                way->user(way_item.user());

                parse_subitems(way_item, way);

                m_handler.way(way);
            }

            void parse_relation(const Osmium::Ser::Relation& relation_item) {
                shared_ptr<Osmium::OSM::Relation> relation = make_shared<Osmium::OSM::Relation>();
                relation->id(relation_item.id());
                relation->version(relation_item.version());
                relation->uid(relation_item.uid());
                relation->changeset(relation_item.changeset());
                relation->timestamp(relation_item.timestamp());
                relation->user(relation_item.user());

                parse_subitems(relation_item, relation);

                m_handler.relation(relation);
            }

            void feed() {
                while (m_offset < m_data.committed()) {
                    const Osmium::Ser::TypedItem& item = m_data.get<Osmium::Ser::TypedItem>(m_offset);
                    if (item.type().is_node()) {
                        parse_node(static_cast<const Osmium::Ser::Node&>(item));
                    } else if (item.type().is_way()) {
                        parse_way(static_cast<const Osmium::Ser::Way&>(item));
                    } else if (item.type().is_relation()) {
                        parse_relation(static_cast<const Osmium::Ser::Relation&>(item));
                    } else {
                        std::cout << "found something else (type=" << item.type() << " length=" << item.size() << ")\n";
                    }
                    m_offset += item.padded_size();
                }
            }

        private:

            const Osmium::Ser::Buffer& m_data;
            size_t m_offset;
            THandler& m_handler;

        }; // class Deserializer

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_DESERIALIZER_HPP
