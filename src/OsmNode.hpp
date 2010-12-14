#ifndef OSMIUM_OSM_NODE_HPP
#define OSMIUM_OSM_NODE_HPP

#include <cmath>

/** @file
*   @brief Contains the Osmium::OSM::Node class.
*/

namespace Osmium {

    namespace OSM {

        class Node : public Object {

        public:

            static const int max_length_coordinate = 12 + 1; ///< maximum length of coordinate string (3 digits + dot + 8 digits + null byte)

            WKBPoint geom;

            char lon_str[max_length_coordinate];
            char lat_str[max_length_coordinate];

        public:

            Node() : Object() {
            }

            void reset() {
                Object::reset();
                lon_str[0] = 0;
                lat_str[0] = 0;
                geom.point.x = NAN;
                geom.point.y = NAN;
            }

            osm_object_type_t type() const {
                return NODE;
            }

            void set_attribute(const char *attr, const char *value) {
                if (!strcmp(attr, "lon")) {
                    if (!memccpy(lon_str, value, 0, max_length_coordinate)) {
                        throw std::length_error("lon value too long");
                    }
                    geom.point.x = atof(lon_str);
                } else if (!strcmp(attr, "lat")) {
                    if (!memccpy(lat_str, value, 0, max_length_coordinate)) {
                        throw std::length_error("lat value too long");
                    }
                    geom.point.y = atof(lat_str);
                } else {
                    Object::set_attribute(attr, value);
                }
            }

            const char *get_lon_str() {
                if (lon_str[0] == '\0') {
                    snprintf(lon_str, max_length_coordinate, "%.7f", geom.point.x);
                }
                return lon_str;
            }

            const char *get_lat_str() {
                if (lat_str[0] == '\0') {
                    snprintf(lat_str, max_length_coordinate, "%.7f", geom.point.y);
                }
                return lat_str;
            }

            double get_lon() const {
                return geom.point.x;
            }

            double get_lat() const {
                return geom.point.y;
            }

        }; // class Node

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_NODE_HPP
