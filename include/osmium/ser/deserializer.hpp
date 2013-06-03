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

// helper for debugging
namespace {
    void hexDump (const char *desc, const void *addr, int len) {
        int i;
        unsigned char buff[17];
        const unsigned char *pc = static_cast<const unsigned char*>(addr);

        // Output description if given.
        if (desc != NULL)
            printf ("%s:\n", desc);

        // Process every byte in the data.
        for (i = 0; i < len; i++) {
            // Multiple of 16 means new line (with line offset).

            if ((i % 16) == 0) {
                // Just don't print ASCII for the zeroth line.
                if (i != 0)
                    printf ("  %s\n", buff);

                // Output the offset.
                printf ("  %04x ", i);
            }

            // Now the hex code for the specific character.
            printf (" %02x", pc[i]);

            // And store a printable ASCII character for later.
            if ((pc[i] < 0x20) || (pc[i] > 0x7e))
                buff[i % 16] = '.';
            else
                buff[i % 16] = pc[i];
            buff[(i % 16) + 1] = '\0';
        }

        // Pad out last line if not exactly 16 characters.
        while ((i % 16) != 0) {
            printf ("   ");
            i++;
        }

        // And print the final ASCII bit.
        printf ("  %s\n", buff);
    }
}

namespace Osmium {

    /* namespace for code related to serializing osm objects */
    namespace Ser {

        template <class THandler>
        class Deserializer {

        public:

            Deserializer(const Osmium::Ser::Buffer& data) : m_data(data), m_offset(0) {
            }

            ~Deserializer() {
            }

            void dump() {
                while (m_offset < m_data.size()) {
                    const size_t length = m_data.get<size_t>(m_offset);
                    const Osmium::Ser::Object& item = m_data.get<Osmium::Ser::Object>(m_offset+sizeof(size_t));
                    std::cout << "object type=" << item.type << " id=" << item.id << "\n";
                    m_offset += sizeof(size_t) + length;
                }
            }

            void feed(THandler& handler) {
                while (m_offset < m_data.size()) {
//                    hexDump("item", &m_data[m_offset], 120);
                    const size_t length = m_data.get<size_t>(m_offset);
                    const Osmium::Ser::Item& item = m_data.get<Osmium::Ser::Item>(m_offset+sizeof(size_t));
                    if (item.type == 'n') {
                        const Osmium::Ser::Node& node_item = static_cast<const Osmium::Ser::Node&>(item);
                        shared_ptr<Osmium::OSM::Node> node = make_shared<Osmium::OSM::Node>();
                        node->id(node_item.id);
                        node->version(node_item.version);
                        node->uid(node_item.uid);
                        node->changeset(node_item.changeset);
                        node->timestamp(node_item.timestamp);
                        node->position(node_item.pos);
                        node->user(node_item.user());

                        Osmium::Ser::Tags tags(node_item);
                        for (Osmium::Ser::TagsIter it = tags.begin(); it != tags.end(); ++it) {
                            node->tags().add(it->key(), it->value());
                        }

                        handler.node(node);
                    } else if (item.type == 'w') {
                        const Osmium::Ser::Way& way_item = static_cast<const Osmium::Ser::Way&>(item);
                        shared_ptr<Osmium::OSM::Way> way = make_shared<Osmium::OSM::Way>();
                        way->id(way_item.id);
                        way->version(way_item.version);
                        way->uid(way_item.uid);
                        way->changeset(way_item.changeset);
                        way->timestamp(way_item.timestamp);
                        way->user(way_item.user());

                        Osmium::Ser::Tags tags(way_item);
                        for (Osmium::Ser::TagsIter it = tags.begin(); it != tags.end(); ++it) {
                            way->tags().add(it->key(), it->value());
                        }

                        Osmium::Ser::Nodes nodes(way_item);
                        for (Osmium::Ser::NodesIter it = nodes.begin(); it != nodes.end(); ++it) {
                            way->nodes().add(*it);
                        }

                        handler.way(way);
                    } else if (item.type == 'r') {
                        const Osmium::Ser::Relation& relation_item = static_cast<const Osmium::Ser::Relation&>(item);
                        shared_ptr<Osmium::OSM::Relation> relation = make_shared<Osmium::OSM::Relation>();
                        relation->id(relation_item.id);
                        relation->version(relation_item.version);
                        relation->uid(relation_item.uid);
                        relation->changeset(relation_item.changeset);
                        relation->timestamp(relation_item.timestamp);
                        relation->user(relation_item.user());

                        Osmium::Ser::Tags tags(relation_item);
                        for (Osmium::Ser::TagsIter it = tags.begin(); it != tags.end(); ++it) {
                            relation->tags().add(it->key(), it->value());
                        }

                        Osmium::Ser::RelationMembers members(relation_item);
                        for (Osmium::Ser::RelationMembersIter it = members.begin(); it != members.end(); ++it) {
                            relation->add_member(it->type, it->ref, it->role());
                        }

                        handler.relation(relation);
                    } else {
                        std::cout << "found something else (length=" << length << ")\n";
                    }
                    m_offset += sizeof(size_t) + length;
                }
            }

        private:

            const Osmium::Ser::Buffer& m_data;
            size_t m_offset;

        }; // class Deserializer

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_DESERIALIZER_HPP
