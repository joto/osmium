#ifndef OSMIUM_JAVASCRIPT_WRAPPER_HPP
#define OSMIUM_JAVASCRIPT_WRAPPER_HPP

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

namespace Osmium {

    namespace Javascript {

        namespace WrapperTemplate {

            struct OSMPosition {

                static v8::Handle<v8::Array> to_array(const Osmium::OSM::Position& position) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> array = v8::Array::New(2);
                    array->Set(0, v8::Number::New(position.lon()));
                    array->Set(1, v8::Number::New(position.lat()));
                    return scope.Close(array);
                }

            };

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

                Geometry() : Osmium::Javascript::Template() {
                    js_template->Set("toWKT",    v8::FunctionTemplate::New(function_template_<Osmium::Geometry::Geometry, to_wkt>));
                    js_template->Set("toWKB",    v8::FunctionTemplate::New(function_template_<Osmium::Geometry::Geometry, to_wkb>));
                    js_template->Set("toHexWKB", v8::FunctionTemplate::New(function_template_<Osmium::Geometry::Geometry, to_hexwkb>));
                }

            };

            struct GeometryNull : public Osmium::Javascript::Template {

                GeometryNull() : Osmium::Javascript::Template() {
                    js_template->Set("toWKT",    v8::FunctionTemplate::New(function_template<Osmium::Javascript::Template, &Osmium::Javascript::Template::js_undefined>));
                    js_template->Set("toWKB",    v8::FunctionTemplate::New(function_template<Osmium::Javascript::Template, &Osmium::Javascript::Template::js_undefined>));
                    js_template->Set("toHexWKB", v8::FunctionTemplate::New(function_template<Osmium::Javascript::Template, &Osmium::Javascript::Template::js_undefined>));
                    js_template->Set("toArray",  v8::FunctionTemplate::New(function_template<Osmium::Javascript::Template, &Osmium::Javascript::Template::js_undefined>));
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

                GeometryPoint() : Geometry() {
                    js_template->SetAccessor(v8::String::NewSymbol("lon"), accessor_getter_<Osmium::Geometry::Point, lon>);
                    js_template->SetAccessor(v8::String::NewSymbol("lat"), accessor_getter_<Osmium::Geometry::Point, lat>);
                    js_template->Set("toArray", v8::FunctionTemplate::New(function_template_<Osmium::Geometry::Point, to_array>));
                }

            };

            struct GeometryLineString : public Geometry {

                static v8::Handle<v8::Value> to_array(const v8::Arguments& /*args*/, Osmium::Geometry::LineString* ls) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> linestring = v8::Array::New(ls->nodes()->size());
                    unsigned int max = ls->nodes()->size() - 1;
                    const Osmium::OSM::WayNodeList& wnl = *(ls->nodes());
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

                GeometryLineString() : Geometry() {
                    js_template->Set("toArray", v8::FunctionTemplate::New(function_template_<Osmium::Geometry::LineString, to_array>));
                }

            };

            struct GeometryPolygon : public Geometry {

                static v8::Handle<v8::Value> to_array(const v8::Arguments& /*args*/, Osmium::Geometry::Polygon* p) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> polygon = v8::Array::New(1);
                    v8::Local<v8::Array> linear_ring = v8::Array::New(p->nodes()->size());
                    polygon->Set(0, linear_ring);
                    unsigned int max = p->nodes()->size() - 1;
                    const Osmium::OSM::WayNodeList& wnl = *(p->nodes());
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

                GeometryPolygon() : Geometry() {
                    js_template->Set("toArray", v8::FunctionTemplate::New(function_template_<Osmium::Geometry::Polygon, to_array>));
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

                static v8::Handle<v8::Value> to_array(const v8::Arguments& /*args*/, Osmium::Geometry::MultiPolygon* mp) {
                    v8::HandleScope scope;
                    geos::geom::Geometry* geometry = mp->area()->get_geometry();

                    if (geometry->getGeometryTypeId() == geos::geom::GEOS_MULTIPOLYGON) {
                        v8::Local<v8::Array> multipolygon_array = v8::Array::New(geometry->getNumGeometries());

                        for (size_t i=0; i < geometry->getNumGeometries(); ++i) {
                            const geos::geom::Polygon* polygon = dynamic_cast<const geos::geom::Polygon*>(geometry->getGeometryN(i));
                            v8::Local<v8::Array> polygon_array = v8::Array::New(polygon->getNumInteriorRing());
                            multipolygon_array->Set(i, polygon_array);
                            polygon_array->Set(0, ring_as_array(polygon->getExteriorRing()));
                            for (size_t j=0; j < polygon->getNumInteriorRing(); ++j) {
                                polygon_array->Set(j+1, ring_as_array(polygon->getInteriorRingN(j)));
                            }
                        }
                        return scope.Close(multipolygon_array);
                    } else if (geometry->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
                        const Osmium::OSM::AreaFromWay* area_from_way = dynamic_cast<const Osmium::OSM::AreaFromWay*>(mp->area());
                        if (area_from_way) {
                            v8::Local<v8::Array> polygon = v8::Array::New(1);
                            v8::Local<v8::Array> ring = v8::Array::New(area_from_way->nodes().size());
                            int n = 0;
                            for (Osmium::OSM::WayNodeList::const_iterator it = area_from_way->nodes().begin(); it != area_from_way->nodes().end(); ++it) {
                                v8::Local<v8::Array> coord = v8::Array::New(2);
                                coord->Set(0, v8::Number::New(it->lon()));
                                coord->Set(1, v8::Number::New(it->lat()));
                                ring->Set(n++, coord);
                            }
                            polygon->Set(0, ring);
                            return scope.Close(polygon);
                        }
                    }

                    return scope.Close(v8::Undefined());
                }

                GeometryMultiPolygon() : Geometry() {
                    js_template->Set("toArray", v8::FunctionTemplate::New(function_template_<Osmium::Geometry::MultiPolygon, to_array>));
                }

            };

            struct OSMTagList : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> get_tag_value_by_key(v8::Local<v8::String> property, Osmium::OSM::TagList* tag_list) {
                    const char* key = Osmium::v8_String_to_utf8<Osmium::OSM::Tag::max_utf16_length_key>(property);
                    const char* value = tag_list->get_tag_by_key(key);
                    if (value) {
                        return Osmium::utf8_to_v8_String<Osmium::OSM::Tag::max_utf16_length_value>(value);
                    }
                    return v8::Undefined();
                }

                static v8::Handle<v8::Array> enumerate_tag_keys(Osmium::OSM::TagList* tag_list) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> array = v8::Array::New(tag_list->size());

                    Osmium::OSM::TagList::const_iterator end = tag_list->end();
                    int i = 0;
                    for (Osmium::OSM::TagList::const_iterator it = tag_list->begin(); it != end; ++it) {
                        array->Set(i++, Osmium::utf8_to_v8_String<Osmium::OSM::Tag::max_utf16_length_key>(it->key()));
                    }

                    return scope.Close(array);
                }

                OSMTagList() : Osmium::Javascript::Template() {
                    js_template->SetNamedPropertyHandler(
                        named_property_getter_<Osmium::OSM::TagList, get_tag_value_by_key>,
                        0,
                        0,
                        0,
                        property_enumerator_<Osmium::OSM::TagList, enumerate_tag_keys>
                    );
                }

            };

            struct OSMObject : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> id(Osmium::OSM::Object* object) {
                    return v8::Number::New(object->id());
                }

                static v8::Handle<v8::Value> version(Osmium::OSM::Object* object) {
                    return v8::Integer::New(object->version());
                }

                static v8::Handle<v8::Value> timestamp_as_string(Osmium::OSM::Object* object) {
                    return v8::String::New(object->timestamp_as_string().c_str());
                }

                static v8::Handle<v8::Value> uid(Osmium::OSM::Object* object) {
                    return v8::Integer::New(object->uid());
                }

                static v8::Handle<v8::Value> user(Osmium::OSM::Object* object) {
                    return Osmium::utf8_to_v8_String<Osmium::OSM::Object::max_utf16_length_username>(object->user());
                }

                static v8::Handle<v8::Value> changeset(Osmium::OSM::Object* object) {
                    return v8::Number::New(object->changeset());
                }

                static v8::Handle<v8::Value> visible(Osmium::OSM::Object* object) {
                    return v8::Boolean::New(object->visible());
                }

                static v8::Handle<v8::Value> tags(Osmium::OSM::Object* object) {
                    return OSMTagList::get<OSMTagList>().create_instance((void*)&(object->tags()));
                }

                OSMObject() : Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("id"),        accessor_getter_<Osmium::OSM::Object, id>);
                    js_template->SetAccessor(v8::String::NewSymbol("version"),   accessor_getter_<Osmium::OSM::Object, version>);
                    js_template->SetAccessor(v8::String::NewSymbol("timestamp"), accessor_getter_<Osmium::OSM::Object, timestamp_as_string>);
                    js_template->SetAccessor(v8::String::NewSymbol("uid"),       accessor_getter_<Osmium::OSM::Object, uid>);
                    js_template->SetAccessor(v8::String::NewSymbol("user"),      accessor_getter_<Osmium::OSM::Object, user>);
                    js_template->SetAccessor(v8::String::NewSymbol("changeset"), accessor_getter_<Osmium::OSM::Object, changeset>);
                    js_template->SetAccessor(v8::String::NewSymbol("tags"),      accessor_getter_<Osmium::OSM::Object, tags>);
                    js_template->SetAccessor(v8::String::NewSymbol("visible"),   accessor_getter_<Osmium::OSM::Object, visible>);
                }

            };

            struct OSMNode : public OSMObject {

                static v8::Handle<v8::Value> get_geom(Osmium::OSM::Node* node) {
                    Osmium::Geometry::Point* geom = new Osmium::Geometry::Point(*node);
                    return Osmium::Javascript::Template::get<Osmium::Javascript::WrapperTemplate::GeometryPoint>().create_persistent_instance<Osmium::Geometry::Point>(geom);
                }

                OSMNode() : OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("geom"), accessor_getter_<Osmium::OSM::Node, get_geom>);
                }

            };

            struct OSMWay : public OSMObject {

                static v8::Handle<v8::Value> nodes(Osmium::OSM::Way* way) {
                    return way->nodes().js_instance();
                }

                static v8::Handle<v8::Value> geom(Osmium::OSM::Way* way) {
                    if (way->nodes().has_position()) {
                        Osmium::Geometry::LineString* geom = new Osmium::Geometry::LineString(*way);
                        return Osmium::Javascript::Template::get<Osmium::Javascript::WrapperTemplate::GeometryLineString>().create_persistent_instance<Osmium::Geometry::LineString>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<GeometryNull>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                static v8::Handle<v8::Value> reverse_geom(Osmium::OSM::Way* way) {
                    if (way->nodes().has_position()) {
                        Osmium::Geometry::LineString* geom = new Osmium::Geometry::LineString(*way, true);
                        return Osmium::Javascript::Template::get<Osmium::Javascript::WrapperTemplate::GeometryLineString>().create_persistent_instance<Osmium::Geometry::LineString>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<GeometryNull>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                static v8::Handle<v8::Value> polygon_geom(Osmium::OSM::Way* way) {
                    if (way->nodes().has_position() && way->nodes().is_closed()) {
                        Osmium::Geometry::Polygon* geom = new Osmium::Geometry::Polygon(*way);
                        return Osmium::Javascript::Template::get<Osmium::Javascript::WrapperTemplate::GeometryPolygon>().create_persistent_instance<Osmium::Geometry::Polygon>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<GeometryNull>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                OSMWay() : OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("nodes"),        accessor_getter_<Osmium::OSM::Way, nodes>);
                    js_template->SetAccessor(v8::String::NewSymbol("geom"),         accessor_getter_<Osmium::OSM::Way, geom>);
                    js_template->SetAccessor(v8::String::NewSymbol("reverse_geom"), accessor_getter_<Osmium::OSM::Way, reverse_geom>);
                    js_template->SetAccessor(v8::String::NewSymbol("polygon_geom"), accessor_getter_<Osmium::OSM::Way, polygon_geom>);
                }

            };

            struct OSMRelation : public OSMObject {

                static v8::Handle<v8::Value> members(Osmium::OSM::Relation* relation) {
                    return relation->members().js_instance();
                }

                OSMRelation() : OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("members"), accessor_getter_<Osmium::OSM::Relation, members>);
                }

            };

            struct OSMArea : public OSMObject {

                static v8::Handle<v8::Value> from(Osmium::OSM::Area* area) {
                    const char* value = (area->get_type() == AREA_FROM_WAY) ? "way" : "relation";
                    return v8::String::NewSymbol(value);
                }

                static v8::Handle<v8::Value> geom(Osmium::OSM::Area* area) {
                    if (area->get_geometry()) {
                        Osmium::Geometry::MultiPolygon* geom = new Osmium::Geometry::MultiPolygon(*area);
                        return Osmium::Javascript::Template::get<GeometryMultiPolygon>().create_persistent_instance<Osmium::Geometry::MultiPolygon>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<GeometryNull>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                OSMArea() : OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("from"), accessor_getter_<Osmium::OSM::Area, from>);
                    js_template->SetAccessor(v8::String::NewSymbol("geom"), accessor_getter_<Osmium::OSM::Area, geom>);
                }

            };

        } // namespace WrapperTemplate

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_WRAPPER_HPP
