#ifndef OSMIUM_STORAGE_MEMBER_MAP_VECTOR_HPP
#define OSMIUM_STORAGE_MEMBER_MAP_VECTOR_HPP

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
#include <map>
#include <vector>

#include <boost/foreach.hpp>

#include <osmium/osm/types.hpp>
#include <osmium/ser/utils.hpp>

namespace Osmium {

    namespace Storage {

        namespace Member {

            inline bool vcmp(const std::pair<const osm_object_id_t, osm_object_id_t>& lhs, const std::pair<const osm_object_id_t, osm_object_id_t>& rhs) {
                return (lhs.first == rhs.first && lhs.second < rhs.second) || lhs.first < rhs.first;
            }

            class Vector {

            public:

                typedef std::multimap<const osm_object_id_t, osm_object_id_t> id_map_t;
                typedef std::pair<osm_object_id_t, osm_object_id_t> id_id_t;
                typedef std::vector<id_id_t> v_t;

                Vector() :
                    m_data() {
                }

                Vector(size_t reserve_size) :
                    m_data() {
                    m_data.reserve(reserve_size);
                }

                void unsorted_set(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    set(member_id, object_id);
                }

                void set(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    m_data.push_back(std::make_pair(member_id, object_id));
                }

                std::pair<v_t::iterator, v_t::iterator> get(const osm_object_id_t id) {
                    id_id_t s(id, 0);
                    return std::equal_range(m_data.begin(), m_data.end(), s, vcmp);
                }

                void remove(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    std::pair<v_t::iterator, v_t::iterator> r = get(member_id);
                    for (v_t::iterator it = r.first; it != r.second; ++it) {
                        if (it->second == object_id) {
                            it->second = 0;
                            return;
                        }
                    }
                }

                void dump(int fd) {
                    sort();
                    Osmium::Ser::write(fd, m_data.data(), sizeof(id_id_t) * m_data.size());
                }

                void sort() {
                    std::sort(m_data.begin(), m_data.end(), vcmp);
                }

                void append(const id_map_t::iterator& begin, const id_map_t::iterator& end) {
                    std::copy(begin, end, std::back_inserter(m_data));
                }

            private:

                v_t m_data;

            }; // Vector

        } // namespace Member

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_MEMBER_MAP_VECTOR_HPP
