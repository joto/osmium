#ifndef OSMIUM_STORAGE_MEMBER_MULTIMAP_HPP
#define OSMIUM_STORAGE_MEMBER_MULTIMAP_HPP

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

#include <iostream>
#include <map>
#include <vector>

#include <osmium/osm/types.hpp>
#include <osmium/ser/utils.hpp>

namespace Osmium {

    namespace Storage {

        namespace Member {

            class MultiMap {

            public:

                typedef std::multimap<const osm_object_id_t, osm_object_id_t> collection_type;
                typedef collection_type::iterator iterator;
                typedef collection_type::value_type value_type;

                MultiMap() : m_map() {
                }

                MultiMap(size_t) : m_map() {
                }

                void unsorted_set(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    set(member_id, object_id);
                }

                void set(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    m_map.insert(std::make_pair(member_id, object_id));
                }

                std::pair<iterator, iterator> get(const osm_object_id_t id) {
                    return m_map.equal_range(id);
                }

                void remove(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    std::pair<iterator, iterator> r = get(member_id);
                    for (iterator it = r.first; it != r.second; ++it) {
                        if (it->second == object_id) {
                            m_map.erase(it);
                            return;
                        }
                    }
                }

                iterator begin() {
                    return m_map.begin();
                }

                iterator end() {
                    return m_map.end();
                }

                void clear() {
                    m_map.clear();
                }

                void consolidate() {
                    // intentionally left blank
                }

                typedef std::pair<osm_object_id_t, osm_object_id_t> non_const_value_type;

                void dump(int fd) const {
                    std::vector<non_const_value_type> v;
                    std::copy(m_map.begin(), m_map.end(), std::back_inserter(v));
                    std::sort(v.begin(), v.end());
                    Osmium::Ser::write(fd, &v[0], sizeof(value_type) * v.size());
                }

                size_t size() const {
                    return m_map.size();
                }

            private:
            
                collection_type m_map;

            }; // MultiMap

        } // namespace Member

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_MEMBER_MULTIMAP_HPP
