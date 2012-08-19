#ifndef OSMIUM_OSM_RELATION_MEMBER_HPP
#define OSMIUM_OSM_RELATION_MEMBER_HPP

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

#include <cstring>
#include <stdexcept>
#include <string>

#include <osmium/osm/types.hpp>

namespace Osmium {

    namespace OSM {

        class RelationMember {

        public:

            RelationMember() :
                m_ref(0),
                m_type('x'),
                m_role() {
            }

            static const unsigned int max_characters_role = 255;

            static const unsigned int max_utf16_length_role = 2 * (max_characters_role + 1); ///< maximum number of UTF-16 units

            static const unsigned int max_length_role = 255 * 4 + 1; /* 255 UTF-8 characters + null byte */

            osm_object_id_t ref() const {
                return m_ref;
            }

            RelationMember& ref(osm_object_id_t ref) {
                m_ref = ref;
                return *this;
            }

            char type() const {
                return m_type;
            }

            const char* type_name() const {
                switch (type()) {
                    case 'n':
                        return "node";
                    case 'w':
                        return "way";
                    case 'r':
                        return "relation";
                    default:
                        return "unknown";
                }
            }

            RelationMember& type(char type) {
                m_type = type;
                return *this;
            }

            const char* role() const {
                return m_role.c_str();
            }

            RelationMember& role(const char* role) {
                if (strlen(role) > max_length_role) {
                    throw std::length_error("role too long");
                }
                m_role = role;
                return *this;
            }

        private:

            osm_object_id_t m_ref;
            char            m_type;
            std::string     m_role;

        }; // class RelationMember

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_MEMBER_HPP
