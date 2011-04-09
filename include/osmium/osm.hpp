#ifndef OSMIUM_OSM_HPP
#define OSMIUM_OSM_HPP

#include <stdint.h>

enum osm_object_type_t {
    NODE                       = 1,
    WAY                        = 2,
    RELATION                   = 3,
    MULTIPOLYGON_FROM_WAY      = 4,
    MULTIPOLYGON_FROM_RELATION = 5
};

/*
* The following typedefs are chosen so that they can represent all needed
* numbers and still be reasonably space efficient. As the OSM database is
* growing rapidly, 64 bit ids will be needed at some point!
*/
typedef int32_t osm_object_id_t;    ///< type for OSM object (node, way, or relation) ids
typedef int     osm_version_t;      ///< type for OSM object version number
typedef int32_t osm_changeset_id_t; ///< type for OSM changeset ids
typedef int32_t osm_user_id_t;      ///< type for OSM user ids
typedef int     osm_sequence_id_t;  ///< type for OSM nodes and members sequence ids

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
