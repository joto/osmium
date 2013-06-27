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

#include <osmium/osm/types.hpp>

namespace Osmium {

    namespace Storage {

        namespace Member {

            class MultiMap {

                typedef std::multimap<const osm_object_id_t, osm_object_id_t> id_map_t;

            public:

                MultiMap() : m_map() {
                }

                void set(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    m_map.insert(std::make_pair(member_id, object_id));
                }

                std::pair<id_map_t::const_iterator, id_map_t::const_iterator> get(const osm_object_id_t id) {
                    return m_map.equal_range(id);
                }

                void dump(int fd) const {
                    typedef std::pair<osm_object_id_t, osm_object_id_t> pair_t;
                    std::vector<pair_t> v;
                    std::copy(m_map.begin(), m_map.end(), std::back_inserter(v));
                    Osmium::Ser::write(fd, &v[0], sizeof(pair_t) * v.size());
                }

            private:
            
                id_map_t m_map;

            }; // MultiMap

        } // namespace Member

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_MEMBER_MULTIMAP_HPP
