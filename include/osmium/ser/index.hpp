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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

namespace Osmium {

    namespace Ser {

        namespace Index {

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

            class Null {

            public:

                Null() {
                }

                void set(const osm_object_id_t, const size_t) const {
                    // intentionally left blank
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
                    return m_map[id];
                }

            private:

                std::map<osm_object_id_t, size_t> m_map;

            }; // class Map

            class VectorWithId {

            public:

                VectorWithId() : m_list() {
                }

                void set(const osm_object_id_t id, const size_t offset) {
                    m_list.push_back(list_entry_t(id, offset));
                }

                size_t get(const osm_object_id_t id) {
                    return 0; // XXX
                }

                void dump(int fd) const {
                    ssize_t count = ::write(fd, &m_list[0], sizeof(list_entry_t) * m_list.size());
                    if (count < 0) {
                        throw std::runtime_error("Write error");
                    }
                }

            private:

                std::vector<list_entry_t> m_list;

            }; // class VectorWithId

            class MemWithId {

            public:

                MemWithId(int fd) : m_mem(NULL), m_size(0) {
                    struct stat file_stat;
                    if (::fstat(fd, &file_stat) < 0) {
                        throw std::runtime_error("Can't stat index file");
                    }
                    
                    size_t bufsize = file_stat.st_size;
                    m_mem = reinterpret_cast<list_entry_t*>(::mmap(NULL, bufsize, PROT_READ, MAP_SHARED, fd, 0));
                    if (!m_mem) {
                        throw std::runtime_error("Can't mmap index file");
                    }
                    m_size = bufsize / sizeof(list_entry_t);
                }

                size_t get(const osm_object_id_t id) const {
                    const list_entry_t* e =  std::lower_bound(&m_mem[0], &m_mem[m_size], list_entry_t(id));
                    if (e->id == id) {
                        return e->offset;
                    } else {
                        throw std::runtime_error("not found");
                    }
                }

            private:

                list_entry_t* m_mem;
                size_t m_size;

            }; // class MemWithId

        } // namespace Index

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_INDEX_HPP
