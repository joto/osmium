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

            void parse_node(const Osmium::Ser::Node& node_item) {
                shared_ptr<Osmium::OSM::Node> node = make_shared<Osmium::OSM::Node>();
                node->id(node_item.id);
                node->version(node_item.version);
                node->uid(node_item.uid);
                node->changeset(node_item.changeset);
                node->timestamp(node_item.timestamp);
                node->position(node_item.pos);
                node->user(node_item.user());

                const Osmium::Ser::TagList& tags = *reinterpret_cast<const Osmium::Ser::TagList*>(node_item.tags_position());
                for (Osmium::Ser::TagList::iterator it = tags.begin(); it != tags.end(); ++it) {
                    node->tags().add(it->key(), it->value());
                }

                m_handler.node(node);
            }

            void parse_way(const Osmium::Ser::Way& way_item) {
                shared_ptr<Osmium::OSM::Way> way = make_shared<Osmium::OSM::Way>();
                way->id(way_item.id);
                way->version(way_item.version);
                way->uid(way_item.uid);
                way->changeset(way_item.changeset);
                way->timestamp(way_item.timestamp);
                way->user(way_item.user());

                const Osmium::Ser::TagList& tags = *reinterpret_cast<const Osmium::Ser::TagList*>(way_item.tags_position());
                for (Osmium::Ser::TagList::iterator it = tags.begin(); it != tags.end(); ++it) {
                    way->tags().add(it->key(), it->value());
                }

                const Osmium::Ser::WayNodeList& nodes = *reinterpret_cast<const Osmium::Ser::WayNodeList*>(way_item.members_position());
                for (Osmium::Ser::WayNodeList::iterator it = nodes.begin(); it != nodes.end(); ++it) {
                    way->nodes().add(it->id());
                }

                m_handler.way(way);
            }

            void parse_relation(const Osmium::Ser::Relation& relation_item) {
                shared_ptr<Osmium::OSM::Relation> relation = make_shared<Osmium::OSM::Relation>();
                relation->id(relation_item.id);
                relation->version(relation_item.version);
                relation->uid(relation_item.uid);
                relation->changeset(relation_item.changeset);
                relation->timestamp(relation_item.timestamp);
                relation->user(relation_item.user());

                const Osmium::Ser::TagList& tags = *reinterpret_cast<const Osmium::Ser::TagList*>(relation_item.tags_position());
                for (Osmium::Ser::TagList::iterator it = tags.begin(); it != tags.end(); ++it) {
                    relation->tags().add(it->key(), it->value());
                }

                const Osmium::Ser::RelationMemberList& members = *reinterpret_cast<const Osmium::Ser::RelationMemberList*>(relation_item.members_position());
                for (Osmium::Ser::RelationMemberList::iterator it = members.begin(); it != members.end(); ++it) {
                    relation->add_member(it->type().as_char(), it->ref(), it->role());
                }

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
