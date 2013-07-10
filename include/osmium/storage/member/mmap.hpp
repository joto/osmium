#ifndef OSMIUM_STORAGE_MEMBER_VECTOR_HPP
#define OSMIUM_STORAGE_MEMBER_VECTOR_HPP

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

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <osmium/osm/types.hpp>

namespace Osmium {

    namespace Storage {

        namespace Member {

            inline bool cmp_for_search(const std::pair<const osm_object_id_t, osm_object_id_t>& lhs, const std::pair<const osm_object_id_t, osm_object_id_t>& rhs) {
                return lhs.first < rhs.first;
            }

            class Mmap {

            public:

                typedef std::pair<const osm_object_id_t, osm_object_id_t> value_type;
                typedef value_type* iterator;

                Mmap(int fd) :
                    m_memory(MAP_FAILED),
                    m_memory_size(0),
                    m_data(NULL),
                    m_size(0) {
                    struct stat file_stat;
                    if (::fstat(fd, &file_stat) < 0) {
                        throw std::runtime_error("Can't stat index file");
                    }
                    
                    m_memory_size = file_stat.st_size;
                    m_memory = ::mmap(NULL, m_memory_size, PROT_READ, MAP_SHARED, fd, 0);
                    if (m_memory == MAP_FAILED) {
                        throw std::runtime_error("Can't mmap index file");
                    }
                    m_data = reinterpret_cast<value_type*>(m_memory);
                    m_size = m_memory_size / sizeof(value_type);
                }

                ~Mmap() {
                    if (m_memory != MAP_FAILED) {
                        ::munmap(m_memory, m_memory_size);
                    }
                }

                void dump() {
                    for (iterator it = m_data; it != m_data + m_size; ++it) {
                        std::cout << it->first << ":" << it->second << "\n";
                    }
                }

                std::pair<iterator, iterator> get(const osm_object_id_t id) {
                    return std::equal_range(m_data, m_data+m_size, std::make_pair<const osm_object_id_t, osm_object_id_t>(id, 0), cmp_for_search);
                }

            private:

                void *m_memory;
                size_t m_memory_size;

                value_type* m_data;
                size_t m_size;

            }; // Mmap

        } // namespace Member

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_MEMBER_VECTOR_HPP
