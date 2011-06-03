#ifndef OSMIUM_OSM_HPP
#define OSMIUM_OSM_HPP

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

#include <stdint.h>

enum osm_object_type_t {
    UNKNOWN                    = -1,
    NODE                       = 0,
    WAY                        = 1,
    RELATION                   = 2,
    MULTIPOLYGON_FROM_WAY      = 3,
    MULTIPOLYGON_FROM_RELATION = 4
};

/*
* The following typedefs are chosen so that they can represent all needed
* numbers and still be reasonably space efficient. As the OSM database is
* growing rapidly, 64 bit ids will be needed at some point!
*/
typedef int32_t  osm_object_id_t;    ///< type for OSM object (node, way, or relation) ids
typedef uint32_t osm_version_t;      ///< type for OSM object version number
typedef int32_t  osm_changeset_id_t; ///< type for OSM changeset ids
typedef int32_t  osm_user_id_t;      ///< type for OSM user ids
typedef uint32_t osm_sequence_id_t;  ///< type for OSM nodes and members sequence ids

namespace Osmium {

    /**
    *  @brief Namespace for basic OSM objects such as Tag, Node, ...
    */
    namespace OSM {
    } // namespace OSM

} // namespace Osmium

#include <osmium/osm/tag.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation_member.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/multipolygon.hpp>

#endif // OSMIUM_OSM_HPP
