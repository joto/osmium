#ifndef OSMIUM_SER_ITEM_HPP
#define OSMIUM_SER_ITEM_HPP

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

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <osmium/osm/types.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/tag_list.hpp>
#include <osmium/handler.hpp>

#include <osmium/ser/buffer.hpp>

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

        // all items that are serialized start with this header
        class Item {
        public:
            uint64_t offset;
            char type;
            char padding[7];
        };

        /**
         * Iterator to iterate over tags in a TagList
         */
        class TagListIter {

        public:

            TagListIter(const char* start, const char* end) : m_start(start), m_end(end) {
            }

            TagListIter& operator++() {
                m_start += strlen(m_start) + 1;
                m_start += strlen(m_start) + 1;
                return *this;
            }

            TagListIter operator++(int) {
                TagListIter tmp(*this);
                operator++();
                return tmp;
            }

            bool operator==(const TagListIter& rhs) {return m_start==rhs.m_start;}
            bool operator!=(const TagListIter& rhs) {return m_start!=rhs.m_start;}

            std::pair<const char*, const char*> operator*() {
                return std::make_pair(m_start, m_start + strlen(m_start) + 1);
            }
            
        private:

            const char* m_start;
            const char* m_end;

        }; // class TagListIter

        /**
         * List of tags in a buffer.
         */
        class TagList {

        public:

            TagList(const char* buffer) : m_buffer(buffer+sizeof(length_t)), m_size(*reinterpret_cast<const length_t*>(buffer)) {
            }

            TagListIter begin() {
                return TagListIter(m_buffer, m_buffer + m_size);
            }

            TagListIter end() {
                return TagListIter(m_buffer + m_size, m_buffer + m_size);
            }

        private:

            const char* m_buffer;
            int32_t m_size;

        }; // class TagList

        /**
         * Serializes OSM objects into a given buffer.
         */
        class Serializer {

        public:

            Serializer(Osmium::Ser::Buffer& data) : m_data(data) {
            }

            void add_node(const Osmium::OSM::Node& node) {
                Osmium::Ser::NodeBuilder nb(m_data);

                Osmium::Ser::Node& sn = nb.node();
                sn.offset    = 0;
                sn.id        = node.id();
                sn.version   = node.version();
                sn.timestamp = node.timestamp();
                sn.uid       = node.uid();
                sn.changeset = node.changeset();
                sn.pos       = node.position();

                Osmium::Ser::TagListBuilder tags(m_data, &nb);
                BOOST_FOREACH(const Osmium::OSM::Tag& tag, node.tags()) {
                    tags.add_tag(tag.key(), tag.value());
                }
                tags.done();

            }

        private:

            Osmium::Ser::Buffer& m_data;

        }; // class Serializer

        template <class THandler>
        class Deserializer {

        public:

            Deserializer(const Osmium::Ser::Buffer& data) : m_data(data), m_offset(0) {
            }

            ~Deserializer() {
            }

            void feed(THandler& handler) {
                while (m_offset < m_data.size()) {
                    hexDump("item", &m_data[m_offset], 120);
                    const length_t length = *reinterpret_cast<const length_t*>(&m_data[m_offset]);
                    const Osmium::Ser::Item* item = reinterpret_cast<const Osmium::Ser::Item*>(&m_data[m_offset+sizeof(length_t)]);
                    if (item->type == 'n') {
                        const Osmium::Ser::Node* node_item = reinterpret_cast<const Osmium::Ser::Node*>(item);
                        std::cout << "found node (length=" << length << ")\n";
                        shared_ptr<Osmium::OSM::Node> node = make_shared<Osmium::OSM::Node>();
                        node->id(node_item->id);
                        node->version(node_item->version);
                        node->changeset(node_item->changeset);
                        node->timestamp(node_item->timestamp);
                        node->position(node_item->pos);

                        Osmium::Ser::TagList tags(&m_data[m_offset+sizeof(length_t)+sizeof(Osmium::Ser::Node)]);
                        for (Osmium::Ser::TagListIter it = tags.begin(); it != tags.end(); ++it) {
                            const std::pair<const char*, const char*>& kv = *it;
                            node->tags().add(kv.first, kv.second);
                        }

                        handler.node(node);

                        std::cout << "node end\n";
                    } else {
                        std::cout << "found something else (length=" << length << ")\n";
                    }
                    m_offset += sizeof(length_t) + length;
                }
            }

        private:

            const Osmium::Ser::Buffer& m_data;
            int m_offset;

        }; // class Deserializer

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_ITEM_HPP
