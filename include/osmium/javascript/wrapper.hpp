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
                    return object->tags().js_instance();
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
                    return Osmium::Javascript::Template::get<Osmium::Geometry::Point::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Point>(geom);
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
                        return Osmium::Javascript::Template::get<Osmium::Geometry::LineString::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::LineString>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<Osmium::Geometry::Null::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                static v8::Handle<v8::Value> reverse_geom(Osmium::OSM::Way* way) {
                    if (way->nodes().has_position()) {
                        Osmium::Geometry::LineString* geom = new Osmium::Geometry::LineString(*way, true);
                        return Osmium::Javascript::Template::get<Osmium::Geometry::LineString::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::LineString>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<Osmium::Geometry::Null::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Null>(geom);
                    }
                }

                static v8::Handle<v8::Value> polygon_geom(Osmium::OSM::Way* way) {
                    if (way->nodes().has_position() && way->nodes().is_closed()) {
                        Osmium::Geometry::Polygon* geom = new Osmium::Geometry::Polygon(*way);
                        return Osmium::Javascript::Template::get<Osmium::Geometry::Polygon::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Polygon>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<Osmium::Geometry::Null::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Null>(geom);
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
                        return Osmium::Javascript::Template::get<Osmium::Geometry::MultiPolygon::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::MultiPolygon>(geom);
                    } else {
                        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
                        return Osmium::Javascript::Template::get<Osmium::Geometry::Null::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Null>(geom);
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
