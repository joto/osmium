#ifndef OSMIUM_OSM_NODE_HPP
#define OSMIUM_OSM_NODE_HPP

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

#include <cstdio>
#include <cmath>
#include <sstream>
#include <ostream>

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
#endif // OSMIUM_WITH_SHPLIB

#include <osmium/utils/wkb.hpp>

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
#ifdef OSMIUM_WITH_JAVASCRIPT
                js_tags_instance   = Osmium::Javascript::Template::create_tags_instance(this);
                js_object_instance = Osmium::Javascript::Template::create_node_instance(this);
                js_geom_instance   = Osmium::Javascript::Template::create_node_geom_instance(this);
#endif // OSMIUM_WITH_JAVASCRIPT
            }

            void reset() {
                Object::reset();
                geom.point.x = NAN;
                geom.point.y = NAN;
            }

            osm_object_type_t get_type() const {
                return NODE;
            }

            void set_x(double x) {
                geom.point.x = x;
            }

            void set_y(double y) {
                geom.point.y = y;
            }

            void set_coordinates(double x, double y) {
                geom.point.x = x;
                geom.point.y = y;
            }

            /// get longitude as string, returns a pointer to statically allocated memory thats valid until the next call to get_lon_str()
            const char *get_lon_str() const {
                static char lon_str[max_length_coordinate];
                snprintf(lon_str, max_length_coordinate, "%.7f", geom.point.x);
                return lon_str;
            }

            /// get latitude as string, returns a pointer to statically allocated memory thats valid until the next call to get_lat_str()
            const char *get_lat_str() const {
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

            const char *geom_as_hex_wkb() const {
                return geom.to_hex();
            }

#ifdef OSMIUM_WITH_SHPLIB
            SHPObject *create_shp_point(std::string& /*transformation*/) {
                return SHPCreateSimpleObject(SHPT_POINT, 1, &geom.point.x, &geom.point.y, NULL);
            }
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_geom_instance;

            v8::Handle<v8::Value> js_get_lon() const {
                return v8::String::New(get_lon_str());
            }

            v8::Handle<v8::Value> js_get_lat() const {
                return v8::String::New(get_lat_str());
            }

            v8::Handle<v8::Value> js_get_geom() const {
                return js_geom_instance;
            }

            std::ostream& geom_as_wkt(std::ostream& s) const {
                return s << "POINT(" << get_lon() << " " << get_lat() << ")";
            }

            v8::Handle<v8::Value> js_get_geom_property(v8::Local<v8::String> property) const {
                v8::String::Utf8Value key(property);

                if (!strcmp(*key, "as_wkt")) {
                    std::ostringstream oss;
                    geom_as_wkt(oss);
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_ewkt")) {
                    std::ostringstream oss;
                    oss << "SRID=4326;";
                    geom_as_wkt(oss);
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_hex_wkb")) {
                    std::ostringstream oss;
                    oss << geom_as_hex_wkb();
                    //            } else if (!strcmp(*key, "as_hex_ewkb")) {
                    //                oss << geom.to_hex();             TODO TODO
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_array")) {
                    v8::Local<v8::Array> array = v8::Array::New(2);
                    array->Set(v8::Integer::New(0), v8::Number::New(get_lon()));
                    array->Set(v8::Integer::New(1), v8::Number::New(get_lat()));
                    return array;
                } else if (!strcmp(*key, "lon") || !strcmp(*key, "x")) {
                    return v8::Number::New(get_lon());
                } else if (!strcmp(*key, "lat") || !strcmp(*key, "y")) {
                    return v8::Number::New(get_lat());
                } else {
                    return v8::Undefined();
                }
            }
#endif // OSMIUM_WITH_JAVASCRIPT

        }; // class Node

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_NODE_HPP
