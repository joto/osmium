#ifndef OSMIUM_OSM_HPP
#define OSMIUM_OSM_HPP

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

        /**
        *  An OSM tag. Has static allocation with enough memory to hold key and
        *  value strings.
        */
        class Tag {

          public:

            static const int max_length_key   = 4 * 255 + 1; ///< 255 UTF-8 characters + null byte
            static const int max_length_value = 4 * 255 + 1; ///< 255 UTF-8 characters + null byte

            char key[max_length_key];     ///< Tag key
            char value[max_length_value]; ///< Tag value
          
        };

    } // namespace OSM

} // namespace Osmium

#include "OsmObject.hpp"
#include "OsmNode.hpp"
#include "OsmWay.hpp"
#include "OsmRelation.hpp"
#include "OsmMultipolygon.hpp"

#endif // OSMIUM_OSM_HPP
