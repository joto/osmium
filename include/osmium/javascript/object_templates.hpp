#ifndef OSMIUM_JAVASCRIPT_OBJECT_TEMPLATES_HPP
#define OSMIUM_JAVASCRIPT_OBJECT_TEMPLATES_HPP

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

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Object : public Base {

            protected:

                Object() : Base(1) {
                    js_template->SetAccessor(v8::String::New("id"),        accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_id>);
                    js_template->SetAccessor(v8::String::New("version"),   accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_version>);
                    js_template->SetAccessor(v8::String::New("timestamp"), accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_timestamp_as_string>);
                    js_template->SetAccessor(v8::String::New("uid"),       accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_uid>);
                    js_template->SetAccessor(v8::String::New("user"),      accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_user>);
                    js_template->SetAccessor(v8::String::New("changeset"), accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_changeset>);
                    js_template->SetAccessor(v8::String::New("tags"),      accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_tags>);
                    js_template->SetAccessor(v8::String::New("visible"),   accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_visible>);
                }

            }; // class Object

            class Tags : public Base {

            public:

                Tags() : Base(1) {
                    js_template->SetNamedPropertyHandler(
                        named_property_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_tag_value_by_key>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Object, &Osmium::OSM::Object::js_enumerate_tag_keys>
                    );
                }

            }; // class Tags

            class NodeGeom : public Base {

            public:

                NodeGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(named_property_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_geom_property>);
                }

            }; // class NodeGeom

            class Node : public Object {

            public:

                Node() : Object() {
                    js_template->SetAccessor(v8::String::New("lon"),  accessor_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_lon>);
                    js_template->SetAccessor(v8::String::New("lat"),  accessor_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_lat>);
                    js_template->SetAccessor(v8::String::New("geom"), accessor_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_geom>);
                }

            }; // class Node

            class WayGeom : public Base {

            public:

                WayGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(named_property_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_geom_property>);
                }

            }; // class WayGeom

            class Nodes : public Base {

            public:

                Nodes() : Base(1) {
                    js_template->SetAccessor(v8::String::New("length"), accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_nodes_length>);
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_node_id>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Way, &Osmium::OSM::Way::js_enumerate_nodes>
                    );
                }

            }; // class Nodes

            class Way : public Object {

            public:

                Way() : Object() {
                    js_template->SetAccessor(v8::String::New("nodes"), accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_nodes>);
                    js_template->SetAccessor(v8::String::New("geom"),  accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_geom>);
                }

            }; // class Way

            class Member : public Base {

            public:

                Member() : Base(1) {
                    js_template->SetAccessor(v8::String::New("type"), accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_get_type>);
                    js_template->SetAccessor(v8::String::New("ref"),  accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_get_ref>);
                    js_template->SetAccessor(v8::String::New("role"), accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_get_role>);
                }

            }; // class Member

            class Members : public Base {

            public:

                Members() : Base(1) {
                    js_template->SetAccessor(v8::String::New("length"), accessor_getter<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_get_members_length>);
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_get_member>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_enumerate_members>
                    );
                }

            }; // class Members

            class Relation : public Object {

            public:

                Relation() : Object() {
                    js_template->SetAccessor(v8::String::New("members"), accessor_getter<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_get_members>);
                }

            }; // class Relation

            class Multipolygon : public Object {

            public:

                Multipolygon() : Object() {
                    js_template->SetAccessor(v8::String::New("from"), accessor_getter<Osmium::OSM::Multipolygon, &Osmium::OSM::Multipolygon::js_get_from>);
                    js_template->SetAccessor(v8::String::New("geom"), accessor_getter<Osmium::OSM::Multipolygon, &Osmium::OSM::Multipolygon::js_get_geom>);
                }

            }; // class Multipolygon

            class MultipolygonGeom : public Base {

            public:

                MultipolygonGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(named_property_getter<Osmium::OSM::Multipolygon, &Osmium::OSM::Multipolygon::js_get_geom_property>);
                }

            }; // class MultipolygonGeom

            class OutputCSV : public Base {

            public:

                OutputCSV() : Base(1) {
                    js_template->Set("print", v8::FunctionTemplate::New(function_template<Osmium::Output::CSV, &Osmium::Output::CSV::js_print>));
                    js_template->Set("close", v8::FunctionTemplate::New(function_template<Osmium::Output::CSV, &Osmium::Output::CSV::js_close>));
                }

            }; // class OutputCSV

            class OutputShapefile : public Base {

            public:

                OutputShapefile() : Base(1) {
                    js_template->Set("add_field", v8::FunctionTemplate::New(function_template<Osmium::Output::Shapefile, &Osmium::Output::Shapefile::js_add_field>));
                    js_template->Set("add",       v8::FunctionTemplate::New(function_template<Osmium::Output::Shapefile, &Osmium::Output::Shapefile::js_add>));
                    js_template->Set("close",     v8::FunctionTemplate::New(function_template<Osmium::Output::Shapefile, &Osmium::Output::Shapefile::js_close>));
                }

            }; // class OutputShapefile

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_OBJECT_TEMPLATES_HPP
