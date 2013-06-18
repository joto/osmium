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

namespace Osmium {

    namespace Storage {

        namespace Member {

            class MapVector {

                typedef std::vector<osm_object_id_t> id_vector_t;
                typedef std::map<osm_object_id_t, id_vector_t> id_to_vector_of_ids_map_t;

            public:

                MapVector() : m_map() {
                }

                void set(const osm_object_id_t member_id, const osm_object_id_t object_id) {
                    id_vector_t& v = m_map[member_id];
                    if (std::find(v.begin(), v.end(), object_id) == v.end()) {
                        v.push_back(object_id);
                    }
                }

                id_vector_t& get(const osm_object_id_t id) {
                    return m_map[id];
                }

                void dump(const char* prefix) const {
                    for (id_to_vector_of_ids_map_t::const_iterator it = m_map.begin(); it != m_map.end(); ++it) {
                        std::cout << prefix << it->first << ":"; 
                        BOOST_FOREACH(const osm_object_id_t id, it->second) {
                            std::cout << " " << id;
                        }
                        std::cout << "\n";
                    }
                }

            private:
            
                id_to_vector_of_ids_map_t m_map;

            }; // MapVector

        } // namespace Member

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_MEMBER_MAP_VECTOR_HPP
