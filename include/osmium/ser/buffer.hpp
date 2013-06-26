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

#include <boost/function.hpp>
#include <boost/utility.hpp>

#include <osmium/ser/item.hpp>

namespace Osmium {

    namespace Ser {

        /**
         * Exception thrown by the Buffer class when somebody tries to write data
         * into the buffer and it doesn't fit.
         */
        class BufferIsFull {};

        /**
         * Buffer for serialized OSM objects. Is initialized with memory pointer, size
         * and a callback function that is called when the buffer is full. Buffers are
         * usually created by one of the classes in the BufferManager namespace.
         */
        class Buffer : boost::noncopyable {

        public:

            Buffer(char* data, size_t size) :
                m_data(data),
                m_size(size),
                m_written(size),
                m_committed(size) {
                if (size % align_bytes != 0) {
                    throw std::invalid_argument("buffer size needs to be multiple of alignment");
                }
            }

            Buffer(char* data, size_t size, size_t committed) :
                m_data(data),
                m_size(size),
                m_written(committed),
                m_committed(committed) {
                if (size % align_bytes != 0) {
                    throw std::invalid_argument("buffer size needs to be multiple of alignment");
                }
            }

            char* data() const {
                return m_data;
            }

            size_t size() const {
                return m_size;
            }

            size_t committed() const {
                return m_committed;
            }

            /**
             * This tests if the current state of the buffer is aligned
             * properly. Only used for asserts.
             */
            bool is_aligned() const {
                return (m_written % align_bytes == 0) && (m_committed % align_bytes == 0);
            }

            size_t commit() {
                assert(is_aligned());
                size_t offset = m_committed;
                m_committed = m_written;
                return offset;
            }

            size_t clear() {
                size_t committed = m_committed;
                m_written = 0;
                m_committed = 0;
                return committed;
            }

            /**
             * Reserve space of given size in buffer and return pointer to it.
             */
            char* get_space(size_t size) {
                if (m_written + size > m_size) {
                    throw BufferIsFull();
                }
                char* data = &m_data[m_written];
                m_written += size;
                return data;
            }

            /**
             * Reserve space for an object of class T in buffer and return
             * pointer to it.
             */
            template <class T>
            T* get_space_for() {
                assert(is_aligned());
                assert(sizeof(T) % align_bytes == 0);
                return reinterpret_cast<T*>(get_space(sizeof(T)));
            }

            /**
             * Append \0-terminated string to buffer.
             */
            size_t append(const char* str) {
                size_t length = strlen(str) + 1;
                if (m_written + length > m_size) {
                    throw BufferIsFull();
                }
                memcpy(&m_data[m_written], str, length);
                m_written += length;
                return length;
            }

            void add_item(const Osmium::Ser::TypedItem* item) {
                memcpy(get_space(item->size()), item, item->size()); 
            }

            template <class T>
            T& get(const size_t offset) const {
                return *reinterpret_cast<T*>(&m_data[offset]);
            }

            typedef Osmium::Ser::CollectionIterator<TypedItem> iterator;

            iterator begin() {
                return iterator(m_data);
            }

            iterator end() {
                return iterator(m_data + m_committed);
            }

        private:

            char* m_data;
            const size_t m_size;
            size_t m_written;
            size_t m_committed;

        }; // class Buffer

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_BUFFER_HPP
