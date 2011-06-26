#ifndef OSMIUM_GEOMETRY_MULTIPOLYGON_HPP
#define OSMIUM_GEOMETRY_MULTIPOLYGON_HPP

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

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
#endif // OSMIUM_WITH_SHPLIB

#include <osmium/geometry/from_way.hpp>

namespace Osmium {

    namespace Geometry {

        class MultiPolygon {

        public:

            MultiPolygon(const Osmium::OSM::Multipolygon& mp) : m_mp(&mp) {

            }

#ifdef  OSMIUM_WITH_SHPLIB
            SHPObject *create_shp_object() {
                return NULL;
            }
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_instance() const {
                return JavascriptTemplate::get<JavascriptTemplate>().create_instance((void *)this);
            }

            v8::Handle<v8::Array> js_ring_as_array(const geos::geom::LineString *ring) const {
                const geos::geom::CoordinateSequence *cs = ring->getCoordinatesRO();
                v8::Local<v8::Array> ring_array = v8::Array::New(cs->getSize());
                for (size_t i = 0; i < cs->getSize(); i++) {
                    v8::Local<v8::Array> coord = v8::Array::New(2);
                    coord->Set(v8::Integer::New(0), v8::Number::New(cs->getX(i)));
                    coord->Set(v8::Integer::New(1), v8::Number::New(cs->getY(i)));
                    ring_array->Set(v8::Integer::New(i), coord);
                }

                return ring_array;
            }

            v8::Handle<v8::Value> js_get_property(v8::Local<v8::String> property) const {
                geos::geom::Geometry* geometry = m_mp->get_geometry();

                if (!geometry) {
                    return v8::Undefined();
                }

                v8::String::Utf8Value key(property);
                if (!strcmp(*key, "as_array")) {
                    if (geometry->getGeometryTypeId() == geos::geom::GEOS_MULTIPOLYGON) {
                        v8::Local<v8::Array> multipolygon_array = v8::Array::New(geometry->getNumGeometries());

                        for (size_t i=0; i < geometry->getNumGeometries(); i++) {
                            geos::geom::Polygon *polygon = (geos::geom::Polygon *) geometry->getGeometryN(i);
                            v8::Local<v8::Array> polygon_array = v8::Array::New(polygon->getNumInteriorRing());
                            multipolygon_array->Set(v8::Integer::New(i), polygon_array);
                            polygon_array->Set(v8::Integer::New(0), js_ring_as_array(polygon->getExteriorRing()));
                            for (size_t j=0; j < polygon->getNumInteriorRing(); j++) {
                                polygon_array->Set(v8::Integer::New(j+1), js_ring_as_array(polygon->getInteriorRingN(j)));
                            }
                        }
                        return multipolygon_array;
                    } else {
                        return v8::Undefined();
                    }
                } else {
                    return v8::Undefined();
                }
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template {

                JavascriptTemplate() : Osmium::Javascript::Template() {
                    js_template->SetNamedPropertyHandler(named_property_getter<LineString, &LineString::js_get_property>);
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        private:

            const Osmium::OSM::Multipolygon* m_mp;

        }; // class MultiPolygon

    } // namespace Geometry

} // namespace Osmium

#ifdef OSMIUM_WITH_JAVASCRIPT
v8::Handle<v8::Value> Osmium::OSM::Multipolygon::js_geom() const {
    Osmium::Geometry::MultiPolygon* geom = new Osmium::Geometry::MultiPolygon(*this);
    return Osmium::Javascript::Template::get<Osmium::Geometry::MultiPolygon::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::MultiPolygon>(geom);
}
#endif // OSMIUM_WITH_JAVASCRIPT

#endif // OSMIUM_GEOMETRY_MULTIPOLYGON_HPP
