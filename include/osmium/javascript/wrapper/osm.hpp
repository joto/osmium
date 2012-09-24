#ifndef OSMIUM_JAVASCRIPT_WRAPPER_OSM_HPP
#define OSMIUM_JAVASCRIPT_WRAPPER_OSM_HPP

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
#include <v8.h>

#include <osmium/javascript/unicode.hpp>
#include <osmium/javascript/template.hpp>
#include <osmium/javascript/wrapper/geometry.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/area.hpp>

namespace Osmium {

    namespace Javascript {

        /**
         * @brief Functions wrapping %Osmium objects for use from %Javascript
         */
        namespace Wrapper {

            struct OSMTagList : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> get_value_by_key(v8::Local<v8::String> property, Osmium::OSM::TagList* tag_list) {
                    const char* key = Osmium::v8_String_to_utf8<Osmium::OSM::Tag::max_utf16_length_key>(property);
                    const char* value = tag_list->get_value_by_key(key);
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

                OSMTagList() :
                    Osmium::Javascript::Template() {
                    js_template->SetNamedPropertyHandler(
                        named_property_getter<Osmium::OSM::TagList, get_value_by_key>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::TagList, enumerate_tag_keys>
                    );
                }

            };

            struct OSMWayNodeList : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> length(Osmium::OSM::WayNodeList* wnl) {
                    return v8::Number::New(wnl->size());
                }

                static v8::Handle<v8::Value> get_node_id(uint32_t index, Osmium::OSM::WayNodeList* wnl) {
                    return v8::Number::New((*wnl)[index].ref());
                }

                static v8::Handle<v8::Array> enumerate_nodes(Osmium::OSM::WayNodeList* wnl) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> array = v8::Array::New(wnl->size());

                    for (unsigned int i=0; i < wnl->size(); ++i) {
                        array->Set(i, v8::Integer::New(i));
                    }

                    return scope.Close(array);
                }

                OSMWayNodeList() :
                    Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("length"), accessor_getter<Osmium::OSM::WayNodeList, length>);
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<Osmium::OSM::WayNodeList, get_node_id>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::WayNodeList, enumerate_nodes>
                    );
                }

            };

            struct OSMRelationMember : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> ref(Osmium::OSM::RelationMember* member) {
                    return v8::Number::New(member->ref());
                }

                static v8::Handle<v8::Value> type(Osmium::OSM::RelationMember* member) {
                    char t[2];
                    t[0] = member->type();
                    t[1] = 0;
                    return v8::String::NewSymbol(t);
                }

                static v8::Handle<v8::Value> role(Osmium::OSM::RelationMember* member) {
                    return Osmium::utf8_to_v8_String<Osmium::OSM::RelationMember::max_utf16_length_role>(member->role());
                }

                OSMRelationMember() :
                    Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("type"), accessor_getter<Osmium::OSM::RelationMember, type>);
                    js_template->SetAccessor(v8::String::NewSymbol("ref"),  accessor_getter<Osmium::OSM::RelationMember, ref>);
                    js_template->SetAccessor(v8::String::NewSymbol("role"), accessor_getter<Osmium::OSM::RelationMember, role>);
                }

            };

            struct OSMRelationMemberList : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> get_member(uint32_t index, Osmium::OSM::RelationMemberList* rml) {
                    return OSMRelationMember::get<OSMRelationMember>().create_instance((void*)&((*rml)[index]));
                }

                static v8::Handle<v8::Array> enumerate_members(Osmium::OSM::RelationMemberList* rml) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> array = v8::Array::New(rml->size());

                    for (unsigned int i=0; i < rml->size(); ++i) {
                        array->Set(i, v8::Integer::New(i));
                    }

                    return scope.Close(array);
                }

                static v8::Handle<v8::Value> length(Osmium::OSM::RelationMemberList* rml) {
                    return v8::Number::New(rml->size());
                }

                OSMRelationMemberList() :
                    Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("length"), accessor_getter<Osmium::OSM::RelationMemberList, length>);
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<Osmium::OSM::RelationMemberList, get_member>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::RelationMemberList, enumerate_members>
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

                OSMObject() :
                    Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("id"),        accessor_getter<Osmium::OSM::Object, id>);
                    js_template->SetAccessor(v8::String::NewSymbol("version"),   accessor_getter<Osmium::OSM::Object, version>);
                    js_template->SetAccessor(v8::String::NewSymbol("timestamp"), accessor_getter<Osmium::OSM::Object, timestamp_as_string>);
                    js_template->SetAccessor(v8::String::NewSymbol("uid"),       accessor_getter<Osmium::OSM::Object, uid>);
                    js_template->SetAccessor(v8::String::NewSymbol("user"),      accessor_getter<Osmium::OSM::Object, user>);
                    js_template->SetAccessor(v8::String::NewSymbol("changeset"), accessor_getter<Osmium::OSM::Object, changeset>);
                    js_template->SetAccessor(v8::String::NewSymbol("tags"),      accessor_getter<Osmium::OSM::Object, tags>);
                    js_template->SetAccessor(v8::String::NewSymbol("visible"),   accessor_getter<Osmium::OSM::Object, visible>);
                }

            };

            struct OSMNode : public OSMObject {

                static v8::Handle<v8::Value> get_geom(Osmium::OSM::Node* node) {
                    Osmium::Geometry::Point* geom = new Osmium::Geometry::Point(*node);
                    return GeometryPoint::get<GeometryPoint>().create_persistent_instance<Osmium::Geometry::Point>(geom);
                }

                OSMNode() :
                    OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("geom"), accessor_getter<Osmium::OSM::Node, get_geom>);
                }

            };

            struct OSMWay : public OSMObject {

                static v8::Handle<v8::Value> nodes(Osmium::OSM::Way* way) {
                    return OSMWayNodeList::get<OSMWayNodeList>().create_instance((void*)&(way->nodes()));
                }

                static v8::Handle<v8::Value> geom(Osmium::OSM::Way* way) {
                    if (way->nodes().has_position()) {
                        Osmium::Geometry::LineString* geom = new Osmium::Geometry::LineString(*way);
                        return GeometryLineString::get<GeometryLineString>().create_persistent_instance<Osmium::Geometry::LineString>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return GeometryNull::get<GeometryNull>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                static v8::Handle<v8::Value> reverse_geom(Osmium::OSM::Way* way) {
                    if (way->nodes().has_position()) {
                        Osmium::Geometry::LineString* geom = new Osmium::Geometry::LineString(*way, true);
                        return Osmium::Javascript::Template::get<GeometryLineString>().create_persistent_instance<Osmium::Geometry::LineString>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<GeometryNull>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                static v8::Handle<v8::Value> polygon_geom(Osmium::OSM::Way* way) {
                    if (way->nodes().has_position() && way->nodes().is_closed()) {
                        Osmium::Geometry::Polygon* geom = new Osmium::Geometry::Polygon(*way);
                        return Osmium::Javascript::Template::get<GeometryPolygon>().create_persistent_instance<Osmium::Geometry::Polygon>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<GeometryNull>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                OSMWay() :
                    OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("nodes"),        accessor_getter<Osmium::OSM::Way, nodes>);
                    js_template->SetAccessor(v8::String::NewSymbol("geom"),         accessor_getter<Osmium::OSM::Way, geom>);
                    js_template->SetAccessor(v8::String::NewSymbol("reverse_geom"), accessor_getter<Osmium::OSM::Way, reverse_geom>);
                    js_template->SetAccessor(v8::String::NewSymbol("polygon_geom"), accessor_getter<Osmium::OSM::Way, polygon_geom>);
                }

            };

            struct OSMRelation : public OSMObject {

                static v8::Handle<v8::Value> members(Osmium::OSM::Relation* relation) {
                    return OSMRelationMemberList::get<OSMRelationMemberList>().create_instance((void*)&(relation->members()));
                }

                OSMRelation() :
                    OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("members"), accessor_getter<Osmium::OSM::Relation, members>);
                }

            };

            struct OSMArea : public OSMObject {

                static v8::Handle<v8::Value> from(Osmium::OSM::Area* area) {
                    const char* value = area->from_way() ? "way" : "relation";
                    return v8::String::NewSymbol(value);
                }

                static v8::Handle<v8::Value> geom(Osmium::OSM::Area* area) {
                    Osmium::Geometry::MultiPolygon* geom = new Osmium::Geometry::MultiPolygon(*area);
                    return Osmium::Javascript::Template::get<GeometryMultiPolygon>().create_persistent_instance<Osmium::Geometry::MultiPolygon>(geom);
                }

                OSMArea() :
                    OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("from"), accessor_getter<Osmium::OSM::Area, from>);
                    js_template->SetAccessor(v8::String::NewSymbol("geom"), accessor_getter<Osmium::OSM::Area, geom>);
                }

            };

        } // namespace Wrapper

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_WRAPPER_OSM_HPP
