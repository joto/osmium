#ifndef OSMIUM_JAVASCRIPT_WRAPPER_GEOMETRY_HPP
#define OSMIUM_JAVASCRIPT_WRAPPER_GEOMETRY_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <sstream>
#include <v8.h>

#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>

#include <osmium/javascript/unicode.hpp>
#include <osmium/javascript/template.hpp>
#include <osmium/javascript/wrapper/position.hpp>
#include <osmium/geometry/null.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/geometry/linestring.hpp>
#include <osmium/geometry/polygon.hpp>
#include <osmium/geometry/multipolygon.hpp>

namespace Osmium {

    namespace Javascript {

        namespace Wrapper {

            struct Geometry : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> to_wkt(const v8::Arguments& args, Osmium::Geometry::Geometry* geometry) {
                    std::ostringstream oss;
                    bool with_srid = false;
                    if (args.Length() >= 1) {
                        with_srid = args[0]->ToBoolean()->Value();
                    }
                    oss << geometry->as_WKT(with_srid);
                    return v8::String::New(oss.str().c_str());
                }

                static v8::Handle<v8::Value> to_wkb(const v8::Arguments& args, Osmium::Geometry::Geometry* geometry) {
                    std::ostringstream oss;
                    bool with_srid = false;
                    if (args.Length() >= 1) {
                        with_srid = args[0]->ToBoolean()->Value();
                    }
                    oss << geometry->as_WKB(with_srid);
                    return v8::String::New(oss.str().c_str());
                }

                static v8::Handle<v8::Value> to_hexwkb(const v8::Arguments& args, Osmium::Geometry::Geometry* geometry) {
                    std::ostringstream oss;
                    bool with_srid = false;
                    if (args.Length() >= 1) {
                        with_srid = args[0]->ToBoolean()->Value();
                    }
                    oss << geometry->as_HexWKB(with_srid);
                    return v8::String::New(oss.str().c_str());
                }

                Geometry() :
                    Osmium::Javascript::Template() {
                    js_template->Set("toWKT",    v8::FunctionTemplate::New(function_template<Osmium::Geometry::Geometry, to_wkt>));
                    js_template->Set("toWKB",    v8::FunctionTemplate::New(function_template<Osmium::Geometry::Geometry, to_wkb>));
                    js_template->Set("toHexWKB", v8::FunctionTemplate::New(function_template<Osmium::Geometry::Geometry, to_hexwkb>));
                }

            };

            struct GeometryNull : public Osmium::Javascript::Template {

                GeometryNull() :
                    Osmium::Javascript::Template() {
                    js_template->Set("toWKT",    v8::FunctionTemplate::New(undefined));
                    js_template->Set("toWKB",    v8::FunctionTemplate::New(undefined));
                    js_template->Set("toHexWKB", v8::FunctionTemplate::New(undefined));
                    js_template->Set("toArray",  v8::FunctionTemplate::New(undefined));
                }

            };

            struct GeometryPoint : public Geometry {

                static v8::Handle<v8::Value> lon(Osmium::Geometry::Point* point) {
                    return v8::Number::New(point->lon());
                }

                static v8::Handle<v8::Value> lat(Osmium::Geometry::Point* point) {
                    return v8::Number::New(point->lat());
                }

                static v8::Handle<v8::Value> to_array(const v8::Arguments& /*args*/, Osmium::Geometry::Point* point) {
                    return OSMPosition::to_array(point->position());
                }

                GeometryPoint() :
                    Geometry() {
                    js_template->SetAccessor(v8::String::NewSymbol("lon"), accessor_getter<Osmium::Geometry::Point, lon>);
                    js_template->SetAccessor(v8::String::NewSymbol("lat"), accessor_getter<Osmium::Geometry::Point, lat>);
                    js_template->Set("toArray", v8::FunctionTemplate::New(function_template<Osmium::Geometry::Point, to_array>));
                }

            };

            struct GeometryLineString : public Geometry {

                static v8::Handle<v8::Value> to_array(const v8::Arguments& /*args*/, Osmium::Geometry::LineString* ls) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> linestring = v8::Array::New(ls->nodes().size());
                    unsigned int max = ls->nodes().size() - 1;
                    const Osmium::OSM::WayNodeList& wnl = ls->nodes();
                    if (ls->reverse()) {
                        for (unsigned int i=0; i <= max; ++i) {
                            linestring->Set(max - i, OSMPosition::to_array(wnl[i].position()));
                        }
                    } else {
                        for (unsigned int i=0; i <= max; ++i) {
                            linestring->Set(i, OSMPosition::to_array(wnl[i].position()));
                        }
                    }
                    return scope.Close(linestring);
                }

                GeometryLineString() :
                    Geometry() {
                    js_template->Set("toArray", v8::FunctionTemplate::New(function_template<Osmium::Geometry::LineString, to_array>));
                }

            };

            struct GeometryPolygon : public Geometry {

                static v8::Handle<v8::Value> to_array(const v8::Arguments& /*args*/, Osmium::Geometry::Polygon* p) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> polygon = v8::Array::New(1);
                    v8::Local<v8::Array> linear_ring = v8::Array::New(p->nodes().size());
                    polygon->Set(0, linear_ring);
                    unsigned int max = p->nodes().size() - 1;
                    const Osmium::OSM::WayNodeList& wnl = p->nodes();
                    if (p->reverse()) {
                        for (unsigned int i=0; i <= max; ++i) {
                            linear_ring->Set(max - i, OSMPosition::to_array(wnl[i].position()));
                        }
                    } else {
                        for (unsigned int i=0; i <= max; ++i) {
                            linear_ring->Set(i, OSMPosition::to_array(wnl[i].position()));
                        }
                    }
                    return scope.Close(polygon);
                }

                GeometryPolygon() :
                    Geometry() {
                    js_template->Set("toArray", v8::FunctionTemplate::New(function_template<Osmium::Geometry::Polygon, to_array>));
                }

            };

            struct GeometryMultiPolygon : public Geometry {

                static v8::Handle<v8::Array> ring_as_array(const geos::geom::LineString* ring) {
                    v8::HandleScope scope;
                    const geos::geom::CoordinateSequence* cs = ring->getCoordinatesRO();
                    v8::Local<v8::Array> ring_array = v8::Array::New(cs->getSize());
                    for (size_t i = 0; i < cs->getSize(); ++i) {
                        v8::Local<v8::Array> coord = v8::Array::New(2);
                        coord->Set(0, v8::Number::New(cs->getX(i)));
                        coord->Set(1, v8::Number::New(cs->getY(i)));
                        ring_array->Set(i, coord);
                    }

                    return scope.Close(ring_array);
                }

                static v8::Handle<v8::Value> to_array(const v8::Arguments& /*args*/, Osmium::Geometry::MultiPolygon* multipolygon) {
                    v8::HandleScope scope;
                    const geos::geom::MultiPolygon* geos_multipolygon = multipolygon->borrow_geos_geometry();

                    v8::Local<v8::Array> multipolygon_array = v8::Array::New(geos_multipolygon->getNumGeometries());

                    for (size_t i=0; i < geos_multipolygon->getNumGeometries(); ++i) {
                        const geos::geom::Polygon* polygon = dynamic_cast<const geos::geom::Polygon*>(geos_multipolygon->getGeometryN(i));
                        v8::Local<v8::Array> polygon_array = v8::Array::New(polygon->getNumInteriorRing());
                        multipolygon_array->Set(i, polygon_array);
                        polygon_array->Set(0, ring_as_array(polygon->getExteriorRing()));
                        for (size_t j=0; j < polygon->getNumInteriorRing(); ++j) {
                            polygon_array->Set(j+1, ring_as_array(polygon->getInteriorRingN(j)));
                        }
                    }
                    return scope.Close(multipolygon_array);
                }

                GeometryMultiPolygon() :
                    Geometry() {
                    js_template->Set("toArray", v8::FunctionTemplate::New(function_template<Osmium::Geometry::MultiPolygon, to_array>));
                }

            };

        } // namespace Wrapper

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_WRAPPER_GEOMETRY_HPP
