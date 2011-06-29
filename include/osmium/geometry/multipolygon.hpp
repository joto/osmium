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

#ifdef OSMIUM_WITH_GEOS
# include <geos/io/WKBWriter.h>
#endif // OSMIUM_WITH_GEOS

#include <osmium/geometry.hpp>

namespace Osmium {

    namespace Geometry {

        class MultiPolygon : public Geometry {

        public:

            MultiPolygon(const Osmium::OSM::Area& area) : Geometry(area.get_id()), m_area(&area) {
            }

#ifdef OSMIUM_WITH_GEOS
# ifdef OSMIUM_WITH_SHPLIB
            void dump_geometry(const geos::geom::Geometry *g, std::vector<int>& part_start_list, std::vector<double>& x_list, std::vector<double>& y_list) const {
                switch (g->getGeometryTypeId()) {
                    case geos::geom::GEOS_MULTIPOLYGON:
                    case geos::geom::GEOS_MULTILINESTRING: {
                        for (geos::geom::GeometryCollection::const_iterator it = static_cast<const geos::geom::GeometryCollection*>(g)->begin();
                                it != static_cast<const geos::geom::GeometryCollection*>(g)->end(); ++it) {
                            dump_geometry(*it, part_start_list, x_list, y_list);
                        }
                        break;
                    }
                    case geos::geom::GEOS_POLYGON: {
                        const geos::geom::Polygon* polygon = static_cast<const geos::geom::Polygon*>(g);
                        dump_geometry(polygon->getExteriorRing(), part_start_list, x_list, y_list);
                        for (size_t i=0; i < polygon->getNumInteriorRing(); ++i) {
                            dump_geometry(polygon->getInteriorRingN(i), part_start_list, x_list, y_list);
                        }
                        break;
                    }
                    case geos::geom::GEOS_LINESTRING:
                    case geos::geom::GEOS_LINEARRING: {
                        part_start_list.push_back(x_list.size());
                        const geos::geom::CoordinateSequence *cs = static_cast<const geos::geom::LineString*>(g)->getCoordinatesRO();
                        for (size_t i = 0; i < cs->getSize(); ++i) {
                            x_list.push_back(cs->getX(i));
                            y_list.push_back(cs->getY(i));
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error("invalid geometry type encountered");
                }
            }

            SHPObject *create_shp_object() const {
                if (!m_area->get_geometry()) {
                    throw Osmium::Exception::IllegalGeometry();
                }

                std::vector<double> x_list;
                std::vector<double> y_list;
                std::vector<int> part_start_list;

                dump_geometry(m_area->get_geometry(), part_start_list, x_list, y_list);

                return SHPCreateObject(
                           SHPT_POLYGON,           // nSHPType
                           -1,                     // iShape
                           part_start_list.size(), // nParts
                           &part_start_list[0],    // panPartStart
                           NULL,                   // panPartType
                           x_list.size(),          // nVertices,
                           &x_list[0],             // padfX
                           &y_list[0],             // padfY
                           NULL,                   // padfZ
                           NULL);                  // padfM
            }
# endif // OSMIUM_WITH_SHPLIB

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                geos::io::WKTWriter writer;
                return out << writer.write(m_area->get_geometry());
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.write(*(m_area->get_geometry()), out);
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.writeHEX(*(m_area->get_geometry()), out);
                return out;
            }

# ifdef OSMIUM_WITH_JAVASCRIPT
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
                geos::geom::Geometry* geometry = m_area->get_geometry();

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
                    const Osmium::OSM::AreaFromWay* area_from_way = dynamic_cast<const Osmium::OSM::AreaFromWay*>(m_area);
                    if (area_from_way) {
                        v8::Local<v8::Array> polygon = v8::Array::New(1);
                        v8::Local<v8::Array> ring = v8::Array::New(area_from_way->num_nodes);
                        for (osm_sequence_id_t i=0; i < area_from_way->num_nodes; ++i) {
                            v8::Local<v8::Array> coord = v8::Array::New(2);
                            coord->Set(0, v8::Number::New(area_from_way->lon[i]));
                            coord->Set(1, v8::Number::New(area_from_way->lat[i]));
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
# endif // OSMIUM_WITH_JAVASCRIPT
#endif // OSMIUM_WITH_GEOS

        private:

            const Osmium::OSM::Area* m_area;

        }; // class MultiPolygon

    } // namespace Geometry

} // namespace Osmium

#ifdef OSMIUM_WITH_JAVASCRIPT
v8::Handle<v8::Value> Osmium::OSM::Area::js_geom() const {
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
