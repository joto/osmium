#ifndef OSMIUM_STORAGE_BYID_MMAP_FILE_HPP
#define OSMIUM_STORAGE_BYID_MMAP_FILE_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

#include <osmium/storage/byid.hpp>

namespace Osmium {

    namespace Storage {

        namespace ById {

            /**
            * MmapFile stores data in files using the mmap() system call.
            * It will grow automatically.
            *
            * If you have enough memory it is preferred to use the in-memory
            * version MmapAnon. If you don't have enough memory or want the
            * data to persist, use this version. Note that in any case you need
            * substantial amounts of memory for this to work efficiently.
            */
            template <typename TValue>
            class MmapFile : public Osmium::Storage::ById::Base<TValue> {

            public:

                static const uint64_t size_increment = 10 * 1024 * 1024;

                /**
                * Create mapping backed by file. If filename is empty, a temporary
                * file will be created.
                *
                * @param filename The filename (including the path) for the storage.
                * @param remove Should the file be removed after use?
                * @exception std::bad_alloc Thrown when there is not enough memory or some other problem.
                */
                MmapFile(const std::string& filename="", bool remove=true) :
                    Base<TValue>(),
                    m_size(1) {
                    if (filename == "") {
                        FILE* file = tmpfile();
                        if (!file) {
                            throw std::bad_alloc();
                        }
                        m_fd = fileno(file);
                    } else {
                        m_fd = open(filename.c_str(), O_RDWR | O_CREAT, 0600);
                    }

                    if (m_fd < 0) {
                        throw std::bad_alloc();
                    }

                    // now that the file is open we can immediately remove it
                    // (temporary files are always removed)
                    if (remove && filename != "") {
                        if (unlink(filename.c_str()) < 0) {
                            // XXX what to do here?
                        }
                    }

                    // make sure the file is at least as large as the initial size
                    if (get_file_size() < sizeof(TValue) * m_size) {
                        if (ftruncate(m_fd, sizeof(TValue) * m_size) < 0) {
                            throw std::bad_alloc();
                        }
                    }

                    m_items = static_cast<TValue*>(mmap(NULL, sizeof(TValue) * m_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0));
                    if (m_items == MAP_FAILED) {
                        throw std::bad_alloc();
                    }
                }

                ~MmapFile() {
                    clear();
                }

                void set(const uint64_t id, const TValue value) {
                    if (id >= m_size) {
                        uint64_t new_size = id + size_increment;

                        // if the file backing this mmap is smaller than needed, increase its size
                        if (get_file_size() < sizeof(TValue) * new_size) {
                            if (ftruncate(m_fd, sizeof(TValue) * new_size) < 0) {
                                throw std::bad_alloc();
                            }
                        }

                        if (munmap(m_items, sizeof(TValue) * m_size) < 0) {
                            throw std::bad_alloc();
                        }
                        m_items = static_cast<TValue*>(mmap(NULL, sizeof(TValue) * new_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0));
                        if (m_items == MAP_FAILED) {
                            throw std::bad_alloc();
                        }
                        m_size = new_size;
                    }
                    m_items[id] = value;
                }

                const TValue operator[](const uint64_t id) const {
                    return m_items[id];
                }

                uint64_t size() const {
                    return m_size;
                }

                uint64_t used_memory() const {
                    return m_size * sizeof(TValue);
                }

                void clear() {
                    munmap(m_items, sizeof(TValue) * m_size);
                }

            private:

                uint64_t m_size;

                TValue* m_items;

                int m_fd;

                /// Get file size in bytes.
                uint64_t get_file_size() const {
                    struct stat s;
                    if (fstat(m_fd, &s) < 0) {
                        throw std::bad_alloc();
                    }
                    return s.st_size;
                }

            }; // class MmapFile

        } // namespace ById

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_BYID_MMAP_FILE_HPP
