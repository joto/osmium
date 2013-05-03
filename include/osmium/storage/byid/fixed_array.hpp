#ifndef OSMIUM_STORAGE_BYID_FIXED_ARRAY_HPP
#define OSMIUM_STORAGE_BYID_FIXED_ARRAY_HPP

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

#include <cstdlib>

#include <osmium/storage/byid.hpp>

namespace Osmium {

    namespace Storage {

        namespace ById {

            /**
            * The FixedArray storage stores location in a huge array. The size of
            * the array is given when initializing the object, it must be large
            * enough to hold all items.
            *
            * Only use this store when you know beforehand how many IDs there are.
            * It is mainly provided for cases where the more flexible Mmap storage
            * class does not work.
            *
            * There is no range checking on accessing the store.
            *
            * If you are storing node coordinates, you'll need 8 bytes for each node.
            * At the time of writing this, the largest node ID is about 1.3 billion,
            * so you'll need about 10 GB of memory.
            *
            * Note that this storage class will only work on 64 bit systems if
            * used for storing node coordinates. 32 bit systems just can't address
            * that much memory!
            */
            template <typename TValue>
            class FixedArray : public Osmium::Storage::ById::Base<TValue> {

            public:

                /**
                * Constructor.
                *
                * @param max_id One larger than the largest ID you will ever have.
                * @exception std::bad_alloc Thrown when there is not enough memory.
                */
                FixedArray(const uint64_t max_id) :
                    Base<TValue>(),
                    m_size(max_id) {
                    m_items = static_cast<TValue*>(malloc(sizeof(TValue) * max_id));
                    if (!m_items) {
                        throw std::bad_alloc();
                    }
                }

                ~FixedArray() {
                    clear();
                }

                void set(const uint64_t id, const TValue value) {
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
                    free(m_items);
                    m_items = NULL;
                }

            private:

                uint64_t m_size;

                TValue* m_items;

            }; // class FixedArray

        } // namespace ById

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_BYID_FIXED_ARRAY_HPP
