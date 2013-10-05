#ifndef OSMIUM_STORAGE_BYID_STL_MAP_HPP
#define OSMIUM_STORAGE_BYID_STL_MAP_HPP

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

#include <map>

#include <osmium/storage/byid.hpp>

namespace Osmium {

    namespace Storage {

        namespace ById {

            /**
             * The StlMap storage stores location in an STL map.
             */
            template <typename TValue>
            class StlMap : public Osmium::Storage::ById::Base<TValue> {

            public:

                StlMap() :
                    Osmium::Storage::ById::Base<TValue>(),
                    m_items() {
                }

                ~StlMap() {
                }

                void set(const uint64_t id, const TValue value) {
                    m_items[id] = value;
                }

                const TValue operator[](const uint64_t id) const {
                    return m_items.at(id);
                }

                uint64_t size() const {
                    return m_items.size();
                }

                uint64_t used_memory() const {
                    return 0; // XXX dummy
                }

                void clear() {
                }

            private:

                std::map<uint64_t, TValue> m_items;

            }; // class StlMap

        } // namespace ById

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_BYID_STL_MAP_HPP
