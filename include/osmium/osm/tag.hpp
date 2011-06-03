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

namespace Osmium {

    namespace OSM {

        /**
        *  An OSM tag. Has static allocation with enough memory to hold key and
        *  value strings.
        */
        class Tag {

        public:

            static const int max_characters_key   = 255;
            static const int max_characters_value = 255;

            static const int max_utf16_length_key   = 2 * (max_characters_key   + 1); ///< maximum number of UTF-16 units
            static const int max_utf16_length_value = 2 * (max_characters_value + 1);

            static const int max_length_key   = 4 * max_characters_key   + 1; ///< 255 UTF-8 characters + null byte
            static const int max_length_value = 4 * max_characters_value + 1; ///< 255 UTF-8 characters + null byte

            char key[max_length_key];     ///< Tag key
            char value[max_length_value]; ///< Tag value

        };

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_TAG_HPP
