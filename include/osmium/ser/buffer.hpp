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

namespace Osmium {

    namespace Ser {

        /**
         * Buffer for serialized OSM objects. Is initialized with memory pointer, size
         * and a callback function that is called when the buffer is full. Buffers are
         * usually created by one of the classes in the BufferManager namespace.
         */
        class Buffer : boost::noncopyable {

        public:

            static const int align_to = 8;

            Buffer(char* data, size_t size, boost::function<void()> full_callback) :
                m_data(data),
                m_size(size),
                m_pos(0),
                m_committed(0),
                m_full_callback(full_callback) {
                if (size % align_to != 0) {
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
                if (m_pos + size > m_size) {
                    m_full_callback();
                }
                char* ptr = &m_data[m_pos];
                m_pos += size;
                return ptr;
            }

            template <class T>
            T* get_space_for() {
                assert((m_pos % 8 == 0) && "alignment problem");
                assert(sizeof(T) % 8 == 0 && "alignment problem");
                return reinterpret_cast<T*>(get_space(sizeof(T)));
            }

            template <class T>
            T& get(const size_t offset) const {
                return *reinterpret_cast<T*>(&m_data[offset]);
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

            size_t commit() {
                assert(m_pos % 8 == 0 && "alignment problem");
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
