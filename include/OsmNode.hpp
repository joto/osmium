#ifndef OSMIUM_OSM_NODE_HPP
#define OSMIUM_OSM_NODE_HPP

#include <cmath>

#ifdef WITH_SHPLIB
# include <shapefil.h>
#endif

/** @file
*   @brief Contains the Osmium::OSM::Node class.
*/

namespace Osmium {

    namespace OSM {

        class Node : public Object {

            static const int max_length_coordinate = 12 + 1; ///< maximum length of coordinate string (3 digits + dot + 8 digits + null byte)

            WKBPoint geom;

          public:

            Node() : Object() {
                reset();
#ifdef WITH_JAVASCRIPT
                js_tags_instance   = Osmium::Javascript::Template::create_tags_instance(this);
                js_object_instance = Osmium::Javascript::Template::create_node_instance(this);
                js_geom_instance   = Osmium::Javascript::Template::create_node_geom_instance(this);
#endif
            }

            void reset() {
                Object::reset();
                geom.point.x = NAN;
                geom.point.y = NAN;
            }

            osm_object_type_t get_type() const {
                return NODE;
            }

            void set_attribute(const char *attr, const char *value) {
                if (!strcmp(attr, "lon")) {
                    geom.point.x = atof(value);
                } else if (!strcmp(attr, "lat")) {
                    geom.point.y = atof(value);
                } else {
                    Object::set_attribute(attr, value);
                }
            }

            void set_coordinates(double x, double y) {
                geom.point.x = x;
                geom.point.y = y;
            }

            /// get longitude as string, returns a pointer to statically allocated memory thats valid until the next call to get_lon_str()
            const char *get_lon_str() {
                static char lon_str[max_length_coordinate];
                snprintf(lon_str, max_length_coordinate, "%.7f", geom.point.x);
                return lon_str;
            }

            /// get latitude as string, returns a pointer to statically allocated memory thats valid until the next call to get_lat_str()
            const char *get_lat_str() {
                static char lat_str[max_length_coordinate];
                snprintf(lat_str, max_length_coordinate, "%.7f", geom.point.y);
                return lat_str;
            }

            double get_lon() const {
                return geom.point.x;
            }

            double get_lat() const {
                return geom.point.y;
            }

            char *geom_as_hex_wkb() {
                return geom.to_hex();
            }

#ifdef WITH_SHPLIB
            SHPObject *create_shpobject(int shp_type) {
                if (shp_type != SHPT_POINT) {
                    throw std::runtime_error("a node can only be added to a shapefile of type point");
                }
                return SHPCreateSimpleObject(shp_type, 1, &geom.point.x, &geom.point.y, NULL);
            }
#endif

#ifdef WITH_JAVASCRIPT
            v8::Local<v8::Object> js_geom_instance;
#endif

        }; // class Node

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_NODE_HPP
