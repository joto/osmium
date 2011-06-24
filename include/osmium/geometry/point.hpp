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
#include <osmium/utils/wkb.hpp>

namespace Osmium {

    namespace Geometry {

        class Point : public Geometry {

        public:

            Point(Osmium::OSM::Position& position) : Geometry(), m_position(position) {
            }

            Point(const Osmium::OSM::Node& node) : Geometry() { //, m_position(node.position()) {
                m_position = Osmium::OSM::Position(node.get_lon(), node.get_lat());
            }

            double lon() const {
                return m_position.lon();
            }

            double lat() const {
                return m_position.lat();
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT) const {
                return out << "POINT(" << lon() << " " << lat() << ")";
            }

            std::ostream& write_to_stream(std::ostream& out, AsEWKT) const {
                return out << "SRID=4326;" << this->as_WKT();
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB) const {
                WKBPoint wkb;
                wkb.point.x = lon();
                wkb.point.y = lat();
                return out << wkb.to_hex();
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
                    oss << this->as_EWKT();
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_hex_wkb")) {
                    std::ostringstream oss;
                    oss << this->as_HexWKB();
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_array")) { // used for GeoJSON
                    v8::Local<v8::Array> array = v8::Array::New(2);
                    array->Set(v8::Integer::New(0), v8::Number::New(lon()));
                    array->Set(v8::Integer::New(1), v8::Number::New(lat()));
                    return array;
                } else if (!strcmp(*key, "lon") || !strcmp(*key, "x")) {
                    return v8::Number::New(lon());
                } else if (!strcmp(*key, "lat") || !strcmp(*key, "y")) {
                    return v8::Number::New(lat());
                } else {
                    return v8::Undefined();
                }
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template::Base {

                JavascriptTemplate() : Osmium::Javascript::Template::Base(1) {
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
// XXX this function leads to a resource leak, the created Point object is only available from Javascript
v8::Handle<v8::Value> Osmium::OSM::Node::js_get_geom() const {
    return (new Osmium::Geometry::Point(*this))->js_instance();
}
#endif // OSMIUM_WITH_JAVASCRIPT

#endif // OSMIUM_GEOMETRY_POINT_HPP
