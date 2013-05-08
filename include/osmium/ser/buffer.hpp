#ifndef OSMIUM_SER_BUFFER_HPP
#define OSMIUM_SER_BUFFER_HPP

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

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace Osmium {

    namespace Ser {

        typedef uint64_t length_t;

        /**
         * Buffer for serialized OSM objects. Is initialized with memory pointer and size.
         * Future versions of this might be initialized based on a file to mmap, resize
         * automagically, etc.
         */
        class Buffer {

        public:

            Buffer(char* data, size_t size, boost::function<void()> full_callback) : m_data(data), m_size(size), m_pos(0), m_committed(0), m_full_callback(full_callback) {
            }

            ~Buffer() {
            }

            char* ptr() const {
                return m_data;
            }

            size_t pos() const {
                return m_pos;
            }

            size_t committed() const {
                return m_committed;
            }

            size_t size() const {
                return m_size;
            }

            void clear() {
                m_pos = 0;
                m_committed = 0;
            }

            /**
             * Reserve space of size size in buffer and return pointer to it.
             */
            char* get_space(size_t size) {
                if (m_pos + size > m_size) {
                    m_full_callback();
                }
                char* ptr = &m_data[m_pos];
                m_pos += size;
                return ptr;
            }

            template <class T>
            T* get_space_for() {
                return reinterpret_cast<T*>(get_space(sizeof(T)));
            }

            /**
             * Append \0-terminated string to buffer.
             */
            Buffer& append(const char* str) {
                size_t l = strlen(str) + 1;
                if (m_pos + l > m_size) {
                    m_full_callback();
                }
                memcpy(&m_data[m_pos], str, l);
                m_pos += l;
                return *this;
            }

            const char& operator[](size_t offset) const {
                return m_data[offset];
            }

            void commit() {
                m_committed = m_pos;
            }

        private:

            char* m_data;
            size_t m_size;
            size_t m_pos;
            size_t m_committed;
            boost::function<void()> m_full_callback;

        }; // class Buffer

        namespace BufferManager {

            class Malloc {

            public:
            
                Malloc(size_t size) : m_buffer(0) {
                    char* mem = static_cast<char*>(malloc(size));
                    if (!mem) {
                        throw std::bad_alloc();
                    }
                    m_buffer = new Osmium::Ser::Buffer(mem, size, boost::bind(&Malloc::full, this));
                }

                ~Malloc() {
                    free(m_buffer->ptr());
                    delete m_buffer;
                }

                Osmium::Ser::Buffer& buffer() {
                    return *m_buffer;
                }

                void full() {
                    throw std::range_error("buffer too small");
                }

            private:
            
                Osmium::Ser::Buffer* m_buffer;

            }; // class Malloc

        } // namespace BufferManager

        class Builder {

        public:

            Builder(Buffer& buffer, Builder* parent) : m_buffer(buffer), m_parent(parent) {
                m_size = buffer.get_space_for<length_t>();
                *m_size = 0;
                if (m_parent) {
                    m_parent->add_size(sizeof(length_t));
                }
            }

            virtual add_size(length_t size) {
                *m_size += size;
                if (m_parent) {
                    m_parent->add_size(size);
                }
            }

            length_t size() const {
                return *m_size;
            }

            static const int pad_bytes = 8;

            /**
             * Add padding if needed.
             *
             * Adds size to parent, but not to self!
             */
            void add_padding() {
                length_t mod = size() % pad_bytes;
                if (mod != 0) {
                    m_buffer.get_space(8-mod);
                    if (m_parent) {
                        m_parent->add_size(8-mod);
                    }
                }
            }

        protected:

            Buffer& m_buffer;
            Builder* m_parent;
            length_t* m_size;

        }; // Builder

        class UserNameBuilder : public Builder {

        public:

            UserNameBuilder(Buffer& buffer, Builder* parent=NULL, const char* username=NULL) : Builder(buffer, parent) {
                size_t old_size = m_buffer.pos();
                m_buffer.append(username);
                add_size(m_buffer.pos() - old_size);
                add_padding();
            }

        }; // UserNameBuilder

        class NodeListBuilder : public Builder {

        public:

            NodeListBuilder(Buffer& buffer, Builder* parent=NULL) : Builder(buffer, parent) {
            }

            void add_node(uint64_t ref) {
                uint64_t* nodeidptr = m_buffer.get_space_for<uint64_t>();
                *nodeidptr = ref;
                add_size(sizeof(uint64_t));
            }

            // unfortunately we can't do this in the destructor, because
            // the destructor is not allowed to fail and this might fail
            // if the buffer is full.
            // XXX maybe we can do something clever here?
            void done() {
                add_padding();
            }

        }; // class NodeListBuilder

        class TagListBuilder : public Builder {

        public:

            TagListBuilder(Buffer& buffer, Builder* parent=NULL) : Builder(buffer, parent) {
            }

            void add_tag(const char* key, const char* value) {
                size_t old_size = m_buffer.pos();
                m_buffer.append(key);
                m_buffer.append(value);
                add_size(m_buffer.pos() - old_size);
            }

            // unfortunately we can't do this in the destructor, because
            // the destructor is not allowed to fail and this might fail
            // if the buffer is full.
            // XXX maybe we can do something clever here?
            void done() {
                add_padding();
            }

        }; // class TagListBuilder

        length_t padded_length(length_t length) {
            return (length % 8 == 0) ? length : ((length | 7 ) + 1);
        }

        // any kind of item in a buffer
        class BufferItem {

        public:
        
            BufferItem() {}

            const char* const get_ptr(ptrdiff_t offset) const {
                return reinterpret_cast<const char* const>(this) + offset;
            }

        };

        // serialized form of OSM node
        class Node : public BufferItem {

        public:

            // same as Item from here...
            uint64_t offset;
            char type;
            char padding[7];
            // ...to here

            uint64_t id;
            uint64_t version;
            uint32_t timestamp;
            uint32_t uid;
            uint64_t changeset;
            Osmium::OSM::Position pos;

            const char* const user() const {
                return get_ptr(sizeof(Node) + sizeof(length_t));
            }

            length_t user_length() const {
                return *reinterpret_cast<const length_t*>(get_ptr(sizeof(Node)));
            }

            const char* tags_position() const {
                return get_ptr(sizeof(Node) + sizeof(length_t) + padded_length(user_length()));
            }

        };

        // serialized form of OSM object
        class Object {
        public:

            // same as Item from here...
            uint64_t offset;
            char type;
            char padding[7];
            // ...to here

            uint64_t id;
            uint64_t version;
            uint32_t timestamp;
            uint32_t uid;
            uint64_t changeset;
        };

        class Way : public Object {
        };

        class Relation : public Object {
        };

        class NodeBuilder : public Builder {

        public:

            NodeBuilder(Buffer& buffer, Builder* parent=NULL) : Builder(buffer, parent) {
                m_node = buffer.get_space_for<Node>();
                add_size(sizeof(Node));
                m_node->type = 'n';
            }

            Node& node() {
                return *m_node;
            }

            Node* m_node;

        }; // class NodeBuilder

        class WayBuilder : public Builder {

        public:

            WayBuilder(Buffer& buffer, Builder* parent=NULL) : Builder(buffer, parent) {
                m_way = buffer.get_space_for<Way>();
                add_size(sizeof(Way));
                m_way->type = 'w';
            }

            Way& way() {
                return *m_way;
            }

            Way* m_way;

        }; // class WayBuilder

        class RelationBuilder : public Builder {

        public:

            RelationBuilder(Buffer& buffer, Builder* parent=NULL) : Builder(buffer, parent) {
                m_relation = buffer.get_space_for<Relation>();
                add_size(sizeof(Relation));
                m_relation->type = 'r';
            }

            Relation& relation() {
                return *m_relation;
            }

            Relation* m_relation;

        }; // class RelationBuilder

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_BUFFER_HPP
