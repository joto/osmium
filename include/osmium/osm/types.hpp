#ifndef OSMIUM_OSM_TYPES_HPP
#define OSMIUM_OSM_TYPES_HPP

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

#include <stdint.h>
#include <stdlib.h>

// These type definitions should have been in the Osmium namespace, but it is a
// bit late to change this now...

enum osm_object_type_t {
    UNKNOWN            = -1,
    NODE               = 0,
    WAY                = 1,
    RELATION           = 2,
    AREA               = 3
};

/*
* The following typedefs are chosen so that they can represent all needed
* numbers and still be reasonably space efficient. As the %OSM database is
* growing rapidly, 64 bit IDs are used.
*/
typedef int64_t  osm_object_id_t;    ///< type for %OSM object (node, way, or relation) IDs
typedef uint32_t osm_version_t;      ///< type for %OSM object version number
typedef int32_t  osm_changeset_id_t; ///< type for %OSM changeset IDs
typedef int32_t  osm_user_id_t;      ///< type for %OSM user IDs
typedef uint32_t osm_sequence_id_t;  ///< type for %OSM nodes and members sequence IDs

#ifdef _MSC_VER
# define atoll(x) (_atoi64(x))
#endif

namespace Osmium {

    inline osm_object_id_t string_to_osm_object_id_t(const char* string) {
        return atoll(string);
    }

    inline osm_version_t string_to_osm_version_t(const char* string) {
        return atol(string);
    }

    inline osm_changeset_id_t string_to_osm_changeset_id_t(const char* string) {
        return atol(string);
    }

    inline osm_user_id_t string_to_osm_user_id_t(const char* string) {
        return atol(string);
    }

} // namespace Osmium

#endif // OSMIUM_OSM_TYPES_HPP
