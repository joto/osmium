#ifndef OSMIUM_JAVASCRIPT_HPP
#define OSMIUM_JAVASCRIPT_HPP

#include "v8.h"

#include "JavascriptTemplate.hpp"

namespace Osmium {

    namespace Javascript {

        namespace Object {

            class Wrapper {

                Osmium::OSM::Object *object;

                public:

                v8::Local<v8::Object> js_object_instance;
                v8::Local<v8::Object> js_tags_instance;

                virtual Osmium::OSM::Object *get_object() {
                    return object;
                }

                protected:

                Wrapper() {
                }

                public:

                v8::Local<v8::Object> get_instance() {
                    return js_object_instance;
                }

            }; // class Wrapper

        } // namespace Object

        namespace Node {

            class Wrapper : public Osmium::Javascript::Object::Wrapper {

                public:

                Osmium::OSM::Node *object;

                v8::Local<v8::Object> js_geom_instance;

                Wrapper() : Osmium::Javascript::Object::Wrapper() {
                    object = new Osmium::OSM::Node;
                    object->wrapper = this;
                    js_tags_instance   = Osmium::Javascript::Template::create_tags_instance(this);
                    js_object_instance = Osmium::Javascript::Template::create_node_instance(this);
                    js_geom_instance   = Osmium::Javascript::Template::create_node_geom_instance(this);
                }

                Osmium::OSM::Node *get_object() {
                    return object;
                }

            }; // class Wrapper

        } // namespace Node

        namespace Way {

            class Wrapper : public Osmium::Javascript::Object::Wrapper {

                public:

                Osmium::OSM::Way *object;

                v8::Local<v8::Object> js_nodes_instance;
                v8::Local<v8::Object> js_geom_instance;

                Wrapper() : Osmium::Javascript::Object::Wrapper() {
                    object = new Osmium::OSM::Way;
                    object->wrapper = this;

                    js_tags_instance   = Osmium::Javascript::Template::create_tags_instance(this);
                    js_object_instance = Osmium::Javascript::Template::create_way_instance(this);
                    js_nodes_instance  = Osmium::Javascript::Template::create_way_nodes_instance(this);
                    js_geom_instance   = Osmium::Javascript::Template::create_way_geom_instance(this);
                }

                Osmium::OSM::Way *get_object() {
                    return object;
                }

            }; // class Wrapper

        } // namespace Way

        namespace Relation {

            class Wrapper : public Osmium::Javascript::Object::Wrapper {

                public:

                Osmium::OSM::Relation *object;

                v8::Local<v8::Object> js_members_instance;

                Wrapper() : Osmium::Javascript::Object::Wrapper() {
                    object = new Osmium::OSM::Relation;
                    object->wrapper = this;

                    js_tags_instance    = Osmium::Javascript::Template::create_tags_instance(this);
                    js_object_instance  = Osmium::Javascript::Template::create_relation_instance(this);
                    js_members_instance = Osmium::Javascript::Template::create_relation_members_instance(this);
                }

                Osmium::OSM::Relation *get_object() {
                    return object;
                }

            }; // class Wrapper

        } // namespace Relation

        namespace Multipolygon {

            class Wrapper : public Osmium::Javascript::Object::Wrapper {

                public:

                Osmium::OSM::Multipolygon *object;

                Wrapper(Osmium::OSM::Multipolygon *mp) : Osmium::Javascript::Object::Wrapper() {
                    object = mp;
                    object->wrapper = this;

                    js_tags_instance    = Osmium::Javascript::Template::create_tags_instance(this);
                    js_object_instance  = Osmium::Javascript::Template::create_multipolygon_instance(this);
                }

                Osmium::OSM::Multipolygon *get_object() {
                    return object;
                }

            }; // class Wrapper

        } // namespace Multipolygon

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_HPP
