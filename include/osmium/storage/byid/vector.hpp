#ifndef OSMIUM_STORAGE_BYID_VECTOR_HPP
#define OSMIUM_STORAGE_BYID_VECTOR_HPP

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

#include <algorithm>
#include <utility>
#include <vector>

#include <osmium/osm/types.hpp>
#include <osmium/storage/byid.hpp>

namespace Osmium {

    namespace Storage {

        namespace ById {

            /**
            * This class uses a vector of ID/Value pairs to store the
            * data. The vector must be filled ordered by ID (OSM files
            * are generally ordered that way, so thats usually not a
            * problem). Lookup uses a binary search.
            *
            * This has very low memory overhead for small OSM datasets.
            */
            template <typename TValue>
            class Vector : public Osmium::Storage::ById::Base<TValue> {

                struct item_t {
                    osm_object_id_t id;
                    TValue value;

                    item_t(osm_object_id_t i, TValue v = TValue()) :
                        id(i),
                        value(v) {
                    }

                    bool operator<(const item_t& other) const {
                        return this->id < other.id;
                    }

                    bool operator==(const item_t& other) const {
                        return this->id == other.id;
                    }

                    bool operator!=(const item_t& other) const {
                        return !(*this == other);
                    }
                };

                typedef std::vector<item_t> item_vector_t;
                typedef typename item_vector_t::const_iterator item_vector_it_t;

            public:

                Vector() :
                    Base<TValue>(),
                    m_items() {
                }

                void set(const uint64_t id, const TValue value) {
                    m_items.push_back(item_t(id, value));
                }

                const TValue operator[](const uint64_t id) const {
                    const item_t item(id);
                    const item_vector_it_t result = std::lower_bound(m_items.begin(), m_items.end(), item);
                    if (result == m_items.end() || *result != item) {
                        return TValue(); // nothing found
                    } else {
                        return result->value;
                    }
                }

                uint64_t size() const {
                    return m_items.size();
                }

                uint64_t used_memory() const {
                    return size() * sizeof(item_t);
                }

                void clear() {
                    item_vector_t().swap(m_items);
                }

            private:

                item_vector_t m_items;

            }; // class Vector

        } // namespace ById

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_BYID_VECTOR_HPP
