#ifndef OSMIUM_OSM_TAG_HPP
#define OSMIUM_OSM_TAG_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

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

namespace Osmium {

    namespace OSM {

        /**
        *  An OSM tag. Has static allocation with enough memory to hold key and
        *  value strings.
        */
        class Tag {

        public:

            static const int max_utf16_length_key   = 2 * (255 + 1); ///< maximum number of UTF-16 units
            static const int max_utf16_length_value = 2 * (255 + 1);

            Tag(const char* key, const char* value) : m_key(key), m_value(value) {
            }

            const char* key() const {
                return m_key.c_str();
            }

            const char* value() const {
                return m_value.c_str();
            }

        private:

            std::string m_key;
            std::string m_value;

        };

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_TAG_HPP
