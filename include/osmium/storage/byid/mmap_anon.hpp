#ifndef OSMIUM_STORAGE_BYID_MMAP_ANON_HPP
#define OSMIUM_STORAGE_BYID_MMAP_ANON_HPP

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

#ifdef __linux__

#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <osmium/storage/byid.hpp>

namespace Osmium {

    namespace Storage {

        namespace ById {

            /**
            * MmapAnon stores data in memory using the mmap() system call.
            * It will grow automatically.
            *
            * This does not work on Mac OS X, because it doesn't support mremap().
            * Use MmapFile or FixedArray on Mac OS X instead.
            *
            * If you have enough memory it is preferred to use this in-memory
            * version. If you don't have enough memory or want the data to
            * persist, use the file-backed version MmapFile. Note that in any
            * case you need substantial amounts of memory for this to work
            * efficiently.
            */
            template <typename TValue>
            class MmapAnon : public Osmium::Storage::ById::Base<TValue> {

            public:

                static const uint64_t size_increment = 10 * 1024 * 1024;

                /**
                * Create anonymous mapping without a backing file.
                * @exception std::bad_alloc Thrown when there is not enough memory.
                */
                MmapAnon() :
                    Base<TValue>(),
                    m_size(size_increment) {
                    m_items = static_cast<TValue*>(mmap(NULL, sizeof(TValue) * m_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
                    if (m_items == MAP_FAILED) {
                        throw std::bad_alloc();
                    }
                }

                ~MmapAnon() {
                    clear();
                }

                void set(const uint64_t id, const TValue value) {
                    if (id >= m_size) {
                        uint64_t new_size = id + size_increment;

                        m_items = static_cast<TValue*>(mremap(m_items, sizeof(TValue) * m_size, sizeof(TValue) * new_size, MREMAP_MAYMOVE));
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

            }; // class MmapAnon

        } // namespace ById

    } // namespace Storage

} // namespace Osmium

#else
#  warning "Osmium::Storage::ById::MmapAnon only works on Linux!"
#endif // __linux__

#endif // OSMIUM_STORAGE_BYID_MMAP_ANON_HPP
