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

                OSMObject() : Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("id"),        accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_id>);
                    js_template->SetAccessor(v8::String::NewSymbol("version"),   accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_version>);
                    js_template->SetAccessor(v8::String::NewSymbol("timestamp"), accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_timestamp_as_string>);
                    js_template->SetAccessor(v8::String::NewSymbol("uid"),       accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_uid>);
                    js_template->SetAccessor(v8::String::NewSymbol("user"),      accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_user>);
                    js_template->SetAccessor(v8::String::NewSymbol("changeset"), accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_changeset>);
                    js_template->SetAccessor(v8::String::NewSymbol("tags"),      accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_tags>);
                    js_template->SetAccessor(v8::String::NewSymbol("visible"),   accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_visible>);
                }

            };

            struct OSMNode : public OSMObject {

                OSMNode() : OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("geom"), accessor_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_geom>);
                }

            };

            struct OSMWay : public OSMObject {

                OSMWay() : OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("nodes"),        accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_nodes>);
                    js_template->SetAccessor(v8::String::NewSymbol("geom"),         accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_geom>);
                    js_template->SetAccessor(v8::String::NewSymbol("reverse_geom"), accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_reverse_geom>);
                    js_template->SetAccessor(v8::String::NewSymbol("polygon_geom"), accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_polygon_geom>);
                }

            };

            struct OSMRelation : public OSMObject {

                OSMRelation() : OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("members"), accessor_getter<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_members>);
                }

            };

            struct OSMArea : public OSMObject {

                OSMArea() : OSMObject() {
                    js_template->SetAccessor(v8::String::NewSymbol("from"), accessor_getter<Osmium::OSM::Area, &Osmium::OSM::Area::js_from>);
                    js_template->SetAccessor(v8::String::NewSymbol("geom"), accessor_getter<Osmium::OSM::Area, &Osmium::OSM::Area::js_geom>);
                }

            };

        } // namespace WrapperTemplate

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_WRAPPER_HPP
