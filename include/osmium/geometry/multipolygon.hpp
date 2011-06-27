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

#include <osmium/geometry.hpp>

#ifdef OSMIUM_WITH_GEOS
# include <geos/io/WKBWriter.h>
#endif // OSMIUM_WITH_GEOS

namespace Osmium {

    namespace Geometry {

        class MultiPolygon : public Geometry {

        public:

            MultiPolygon(const Osmium::OSM::Multipolygon& mp) : m_mp(&mp) {
            }

#ifdef OSMIUM_WITH_GEOS
            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                geos::io::WKTWriter writer; 
                return out << writer.write(m_mp->get_geometry());
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.write(*(m_mp->get_geometry()), out);
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.writeHEX(*(m_mp->get_geometry()), out);
                return out;
            }
#endif // OSMIUM_WITH_GEOS

#ifdef OSMIUM_WITH_JAVASCRIPT
# ifdef OSMIUM_WITH_GEOS
            v8::Local<v8::Object> js_instance() const {
                return JavascriptTemplate::get<JavascriptTemplate>().create_instance((void *)this);
            }

            v8::Handle<v8::Array> js_ring_as_array(const geos::geom::LineString *ring) const {
                v8::HandleScope scope;
                const geos::geom::CoordinateSequence *cs = ring->getCoordinatesRO();
                v8::Local<v8::Array> ring_array = v8::Array::New(cs->getSize());
                for (size_t i = 0; i < cs->getSize(); ++i) {
                    v8::Local<v8::Array> coord = v8::Array::New(2);
                    coord->Set(0, v8::Number::New(cs->getX(i)));
                    coord->Set(1, v8::Number::New(cs->getY(i)));
                    ring_array->Set(i, coord);
                }

                return scope.Close(ring_array);
            }

            v8::Handle<v8::Value> js_to_array(const v8::Arguments& /*args*/) {
                v8::HandleScope scope;
                geos::geom::Geometry* geometry = m_mp->get_geometry();

                if (geometry->getGeometryTypeId() == geos::geom::GEOS_MULTIPOLYGON) {
                    v8::Local<v8::Array> multipolygon_array = v8::Array::New(geometry->getNumGeometries());

                    for (size_t i=0; i < geometry->getNumGeometries(); ++i) {
                        geos::geom::Polygon *polygon = (geos::geom::Polygon *) geometry->getGeometryN(i);
                        v8::Local<v8::Array> polygon_array = v8::Array::New(polygon->getNumInteriorRing());
                        multipolygon_array->Set(i, polygon_array);
                        polygon_array->Set(0, js_ring_as_array(polygon->getExteriorRing()));
                        for (size_t j=0; j < polygon->getNumInteriorRing(); ++j) {
                            polygon_array->Set(j+1, js_ring_as_array(polygon->getInteriorRingN(j)));
                        }
                    }
                    return scope.Close(multipolygon_array);
                } else if (geometry->getGeometryTypeId() == geos::geom::GEOS_LINESTRING) {
                    const Osmium::OSM::MultipolygonFromWay* mpfw = dynamic_cast<const Osmium::OSM::MultipolygonFromWay*>(m_mp);
                    if (mpfw) {
                        v8::Local<v8::Array> polygon = v8::Array::New(1);
                        v8::Local<v8::Array> ring = v8::Array::New(mpfw->num_nodes);
                        for (osm_sequence_id_t i=0; i < mpfw->num_nodes; ++i) {
                            v8::Local<v8::Array> coord = v8::Array::New(2);
                            coord->Set(0, v8::Number::New(mpfw->lon[i]));
                            coord->Set(1, v8::Number::New(mpfw->lat[i]));
                            ring->Set(i, coord);
                        }
                        polygon->Set(0, ring);
                        return scope.Close(polygon);
                    }
                }

                return scope.Close(v8::Undefined());
            }

            struct JavascriptTemplate : public Osmium::Geometry::Geometry::JavascriptTemplate {

                JavascriptTemplate() : Osmium::Geometry::Geometry::JavascriptTemplate() {
                    js_template->Set("toArray",  v8::FunctionTemplate::New(function_template<MultiPolygon, &MultiPolygon::js_to_array>));
                }

            };
# endif // OSMIUM_WITH_GEOS
#endif // OSMIUM_WITH_JAVASCRIPT

        private:

            const Osmium::OSM::Multipolygon* m_mp;

        }; // class MultiPolygon

    } // namespace Geometry

} // namespace Osmium

#ifdef OSMIUM_WITH_JAVASCRIPT
v8::Handle<v8::Value> Osmium::OSM::Multipolygon::js_geom() const {
    if (get_geometry()) {
        Osmium::Geometry::MultiPolygon* geom = new Osmium::Geometry::MultiPolygon(*this);
        return Osmium::Javascript::Template::get<Osmium::Geometry::MultiPolygon::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::MultiPolygon>(geom);
    } else {
        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
        return Osmium::Javascript::Template::get<Osmium::Geometry::Null::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Null>(geom);
    }
}
#endif // OSMIUM_WITH_JAVASCRIPT

#endif // OSMIUM_GEOMETRY_MULTIPOLYGON_HPP
