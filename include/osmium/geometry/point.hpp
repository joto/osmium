#ifndef OSMIUM_GEOMETRY_POINT_HPP
#define OSMIUM_GEOMETRY_POINT_HPP

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

#include <vector>
#include <sstream>

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
#endif // OSMIUM_WITH_SHPLIB

#include <osmium/geometry.hpp>
#include <osmium/osm/node.hpp>

namespace Osmium {

    namespace Geometry {

        /**
         * Point geometry.
         */
        class Point : public Geometry {

        public:

            /**
             * Create point geometry from a position.
             */
            Point(Osmium::OSM::Position& position) : Geometry(), m_position(position) {
            }

            /**
             * Create point geometry from position of a node.
             */
            Point(const Osmium::OSM::Node& node) : Geometry() { //, m_position(node.position()) {
                m_position = Osmium::OSM::Position(node.get_lon(), node.get_lat());
            }

            double lon() const {
                return m_position.lon();
            }

            double lat() const {
                return m_position.lat();
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                return out << "POINT(" << std::setprecision(10) << lon() << " " << lat() << ")";
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                write_binary_wkb_header(out, with_srid, wkbPoint);
                write_binary<double>(out, lon());
                write_binary<double>(out, lat());
                return out;
            };

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                write_hex_wkb_header(out, with_srid, wkbPoint);
                write_hex<double>(out, lon());
                write_hex<double>(out, lat());
                return out;
            }

#ifdef OSMIUM_WITH_SHPLIB
            SHPObject *create_shp_object() const {
                double x = lon();
                double y = lat();
                return SHPCreateSimpleObject(SHPT_POINT, 1, &x, &y, NULL);
            }
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_instance() const {
                return JavascriptTemplate::get<JavascriptTemplate>().create_instance((void *)this);
            }

            v8::Handle<v8::Value> js_get_property(v8::Local<v8::String> property) const {
                v8::String::Utf8Value key(property);

                if (!strcmp(*key, "as_wkt")) {
                    std::ostringstream oss;
                    oss << this->as_WKT();
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_ewkt")) {
                    std::ostringstream oss;
                    oss << this->as_WKT(true);
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_hex_wkb")) {
                    std::ostringstream oss;
                    oss << this->as_HexWKB();
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_array")) { // used for GeoJSON
                    v8::HandleScope scope;
                    v8::Local<v8::Array> array = v8::Array::New(2);
                    array->Set(0, v8::Number::New(lon()));
                    array->Set(1, v8::Number::New(lat()));
                    return scope.Close(array);
                } else if (!strcmp(*key, "lon")) {
                    return v8::Number::New(lon());
                } else if (!strcmp(*key, "lat")) {
                    return v8::Number::New(lat());
                } else {
                    return v8::Undefined();
                }
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template {

                JavascriptTemplate() : Osmium::Javascript::Template() {
                    js_template->SetNamedPropertyHandler(named_property_getter<Point, &Point::js_get_property>);
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        private:

            Osmium::OSM::Position m_position;

        }; // class Point

    } // namespace Geometry

} // namespace Osmium


#ifdef OSMIUM_WITH_JAVASCRIPT
v8::Handle<v8::Value> Osmium::OSM::Node::js_get_geom() const {
    Osmium::Geometry::Point* geom = new Osmium::Geometry::Point(*this);
    return Osmium::Javascript::Template::get<Osmium::Geometry::Point::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Point>(geom);
}
#endif // OSMIUM_WITH_JAVASCRIPT

#endif // OSMIUM_GEOMETRY_POINT_HPP
