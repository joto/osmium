#ifndef OSMIUM_STORAGE_BYID_SPARSE_TABLE_HPP
#define OSMIUM_STORAGE_BYID_SPARSE_TABLE_HPP

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

#include <google/sparsetable>

#include <osmium/storage/byid.hpp>

namespace Osmium {

    namespace Storage {

        namespace ById {

            /**
            * The SparseTable store stores items in a Google sparsetable,
            * a data structure that can hold sparsly filled tables in a
            * very space efficient way. It will resize automatically.
            *
            * Use this node location store if the ID space is only sparsly
            * populated, such as when working with smaller OSM files (like
            * country extracts).
            *
            * SparseTable needs a 64bit architecture.
            */
            template <typename TValue>
            class SparseTable : public Osmium::Storage::ById::Base<TValue> {

            public:

                /**
                * Constructor.
                *
                * @param grow_size The initial size of the storage (in items).
                *                  The storage will grow by at least this size
                *                  every time it runs out of space.
                */
                SparseTable(const uint64_t grow_size=10000) :
                    Base<TValue>(),
                    m_grow_size(grow_size),
                    m_items(grow_size) {
                    assert(sizeof(typename google::sparsetable<TValue>::size_type) >= 8 && "google::sparsetable needs 64bit machine");
                }

                void set(const uint64_t id, const TValue value) {
                    if (id >= m_items.size()) {
                        m_items.resize(id + m_grow_size);
                    }
                    m_items[id] = value;
                }

                const TValue operator[](const uint64_t id) const {
                    return m_items[id];
                }

                uint64_t size() const {
                    return m_items.size();
                }

                uint64_t used_memory() const {
                    // unused items use 1 bit, used items sizeof(TValue) bytes
                    // http://google-sparsehash.googlecode.com/svn/trunk/doc/sparsetable.html
                    return (m_items.size() / 8) + (m_items.num_nonempty() * sizeof(TValue));
                }

                void clear() {
                    m_items.clear();
                }

            private:

                uint64_t m_grow_size;

                google::sparsetable<TValue> m_items;

            }; // class SparseTable

        } // namespace ById

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_BYID_SPARSE_TABLE_HPP
