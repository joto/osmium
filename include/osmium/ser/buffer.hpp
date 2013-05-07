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

namespace Osmium {

    namespace Ser {

        /**
         * Buffer for serialized OSM objects. Is initialized with memory pointer and size.
         * Future versions of this might be initialized based on a file to mmap, resize
         * automagically, etc.
         */
        class Buffer {

        public:

            Buffer(char* data, size_t size) : m_data(data), m_size(size), m_pos(0) {
            }

            ~Buffer() {
            }

            size_t size() const {
                return m_pos;
            }

            /**
             * Reserve space of size size in buffer and return pointer to it.
             */
            void* get_space(size_t size) {
                if (m_pos + size > m_size) {
                    throw std::range_error("buffer too small");
                }
                void* ptr = &m_data[m_pos];
                m_pos += size;
                return ptr;
            }

            /**
             * Append \0-terminated string to buffer.
             */
            Buffer& append(const char* str) {
                size_t l = strlen(str) + 1;
                if (m_pos + l > m_size) {
                    throw std::range_error("buffer too small");
                }
                memcpy(&m_data[m_pos], str, l);
                m_pos += l;
                return *this;
            }

            const char& operator[](size_t offset) const {
                return m_data[offset];
            }

        private:

            char* m_data;
            size_t m_size;
            size_t m_pos;

        };

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_BUFFER_HPP
