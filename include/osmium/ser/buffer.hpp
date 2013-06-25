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
         * Buffer for serialized OSM objects. Is initialized with memory pointer, size
         * and a callback function that is called when the buffer is full. Buffers are
         * usually created by one of the classes in the BufferManager namespace.
         */
        class Buffer : boost::noncopyable {

        public:

            Buffer(char* data, size_t size) :
                m_data(data),
                m_size(size),
                m_pos(size),
                m_committed(size),
                m_full_callback(NULL) {
                if (size % align_bytes != 0) {
                    throw std::invalid_argument("buffer size needs to be multiple of alignment");
                }
            }

            Buffer(char* data, size_t size, size_t pos, boost::function<void()> full_callback = NULL) :
                m_data(data),
                m_size(size),
                m_pos(pos),
                m_committed(pos),
                m_full_callback(full_callback) {
                if (size % align_bytes != 0) {
                    throw std::invalid_argument("buffer size needs to be multiple of alignment");
                }
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

            /**
             * This tests if the current state of the buffer is aligned
             * properly. Only used for asserts.
             */
            bool is_aligned() const {
                return (m_pos % align_bytes == 0) && (m_committed % align_bytes == 0);
            }

            size_t size() const {
                return m_size;
            }

            size_t clear() {
                size_t committed = m_committed;
                m_pos = 0;
                m_committed = 0;
                return committed;
            }

            /**
             * Reserve space of given size in buffer and return pointer to it.
             */
            char* get_space(size_t size) {
                if (m_pos + size > m_size && m_full_callback) {
                    m_full_callback();
                }
                char* ptr = &m_data[m_pos];
                m_pos += size;
                return ptr;
            }

            template <class T>
            T* get_space_for() {
                assert(is_aligned());
                assert(sizeof(T) % align_bytes == 0);
                return reinterpret_cast<T*>(get_space(sizeof(T)));
            }

            template <class T>
            T& get(const size_t offset) const {
                return *reinterpret_cast<T*>(&m_data[offset]);
            }

            typedef Osmium::Ser::SubItemIterator iterator;

            iterator begin() {
                return iterator(m_data, m_data + m_committed);
            }

            iterator end() {
                return iterator(m_data + m_committed, m_data + m_committed);
            }

            /**
             * Append \0-terminated string to buffer.
             */
            Buffer& append(const char* str) {
                size_t l = strlen(str) + 1;
                if (m_pos + l > m_size && m_full_callback) {
                    m_full_callback();
                }
                memcpy(&m_data[m_pos], str, l);
                m_pos += l;
                return *this;
            }

            size_t commit() {
                assert(is_aligned());
                size_t offset = m_committed;
                m_committed = m_pos;
                return offset;
            }

        private:

            char* m_data;
            size_t m_size;
            size_t m_pos;
            size_t m_committed;
            boost::function<void()> m_full_callback;

        }; // class Buffer

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_BUFFER_HPP
