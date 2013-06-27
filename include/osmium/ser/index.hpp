#ifndef OSMIUM_SER_INDEX_HPP
#define OSMIUM_SER_INDEX_HPP

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
#include <map>
#include <vector>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#include <osmium/osm/types.hpp>
#include <osmium/ser/utils.hpp>

namespace Osmium {

    namespace Ser {

        /**
         * Indexes let us look up the position of an object in a buffer by the
         * ID of that object. There are several different implementation of
         * indexes suitable for different use cases. They differ in:
         * - Memory use
         * - Suitability for dense or for sparse indexes
         * - How they can be serialized to disk / deserialized from disk.
         *
         * XXX Note that there is a log of overlap between these classes and
         * the Osmium::Storage::ByID classes. This needs to be sorted out.
         */
        namespace Index {

            class NotFound : public std::runtime_error {

            public:

                NotFound(osm_object_id_t) : std::runtime_error("object not found: ") {
                }

            }; // class NotFound

            /**
             * Pseudo index.
             * Use this class if you don't need an index, but you
             * need an object that behaves like one.
             */
            class Null {

            public:

                Null() {
                }

                void set(const osm_object_id_t, const size_t) const {
                    // intentionally left blank
                }

                size_t get(const osm_object_id_t id) {
                    throw NotFound(id);
                }

            }; // class Null

            class Map {

            public:

                Map() : m_map() {
                }

                void set(const osm_object_id_t id, const size_t offset) {
                    m_map[id] = offset;
                }

                size_t get(const osm_object_id_t id) {
                    try {
                        return m_map.at(id);
                    } catch (std::out_of_range&) {
                        throw NotFound(id);
                    }
                }

            private:

                std::map<osm_object_id_t, size_t> m_map;

            }; // class Map

            class Vector {

            public:

                Vector() : m_offsets() {
                }

                void set(const osm_object_id_t id, const size_t offset) {
                    if (id >= m_offsets.size()) {
                        m_offsets.resize(id, -1); // use -1 as marker for uninitialized offset
                    }
                    m_offsets[id] = offset;
                }

                size_t get(const osm_object_id_t id) {
                    if (id < m_offsets.size()) {
                        if (m_offsets[id] != -1) {
                            return m_offsets[id];
                        }
                    }
                    throw NotFound(id);
                }

                void dump(int fd) const {
                    Osmium::Ser::write(fd, &m_offsets[0], sizeof(size_t) * m_offsets.size());
                }

            private:

                std::vector<size_t> m_offsets;

            }; // class Vector

            struct list_entry_t {
                osm_object_id_t id;
                size_t offset;

                list_entry_t(osm_object_id_t i, size_t o = 0) :
                    id(i),
                    offset(o) {
                }

                int operator<(const list_entry_t& rhs) const {
                    return this->id < rhs.id;
                }
            };

            // XXX this will currently only work if entries are entered ordered by id
            class VectorWithId {

            public:

                VectorWithId() : m_list() {
                }

                void set(const osm_object_id_t id, const size_t offset) {
                    m_list.push_back(list_entry_t(id, offset));
                }

                size_t get(const osm_object_id_t id) {
                    std::vector<list_entry_t>::iterator it = std::lower_bound(m_list.begin(), m_list.end(), list_entry_t(id));
                    if (it != m_list.end() && it->id == id) {
                        return it->offset;
                    } else {
                        throw NotFound(id);
                    }
                }

                void dump(int fd) const {
                    Osmium::Ser::write(fd, &m_list[0], sizeof(list_entry_t) * m_list.size());
                }

            private:

                std::vector<list_entry_t> m_list;

            }; // class VectorWithId

            class MemMapWithId {

            public:

                MemMapWithId(int fd) :
                    m_memory(MAP_FAILED),
                    m_memory_size(0),
                    m_list(NULL),
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
                    m_list = reinterpret_cast<list_entry_t*>(m_memory);
                    m_size = m_memory_size / sizeof(list_entry_t);
                }

                ~MemMapWithId() {
                    if (m_memory != MAP_FAILED) {
                        ::munmap(m_memory, m_memory_size);
                    }
                }

                size_t get(const osm_object_id_t id) const {
                    const list_entry_t* it = std::lower_bound(&m_list[0], &m_list[m_size], list_entry_t(id));
                    if (it != &m_list[m_size] && it->id == id) {
                        return it->offset;
                    } else {
                        throw NotFound(id);
                    }
                }

            private:

                void *m_memory;
                size_t m_memory_size;

                list_entry_t* m_list;
                size_t m_size;

            }; // class MemWithId

        } // namespace Index

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_INDEX_HPP
