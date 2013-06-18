#ifndef OSMIUM_JAVASCRIPT_HANDLER_HPP
#define OSMIUM_JAVASCRIPT_HANDLER_HPP

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

#include <fstream>
#include <iostream>
#include <string>
#include <v8.h>

#include <osmium/handler.hpp>
#include <osmium/javascript/wrapper/position.hpp>
#include <osmium/javascript/wrapper/geometry.hpp>
#include <osmium/javascript/wrapper/osm.hpp>
#include <osmium/javascript/wrapper/export_csv.hpp>
#include <osmium/javascript/wrapper/export_shapefile.hpp>

extern v8::Persistent<v8::Context> global_context;

namespace Osmium {

    namespace Javascript {

        class Handler : public Osmium::Handler::Base {

            /***
            * Load Javascript file into string
            */
            static std::string load_file(const char* filename) {
                std::ifstream javascript_file(filename, std::ifstream::in);
                if (javascript_file.fail()) {
                    std::cerr << "Can't open file " << filename << std::endl;
                    exit(1);
                }
                std::stringstream buffer;
                buffer << javascript_file.rdbuf();
                return buffer.str();
            }

            static const char* ToCString(const v8::String::Utf8Value& value) {
                return *value ? *value : "<string conversion failed>";
            }

            /**
            * Print Javascript exception to stderr
            */
            static void report_exception(v8::TryCatch* try_catch) {
                v8::HandleScope handle_scope;
                v8::String::Utf8Value exception(try_catch->Exception());
                const char* exception_string = ToCString(exception);

                v8::Handle<v8::Message> message = try_catch->Message();
                if (message.IsEmpty()) {
                    std::cerr << exception_string << std::endl;
                } else {
                    v8::String::Utf8Value filename(message->GetScriptResourceName());
                    std::cerr << *filename << ":" << message->GetLineNumber() << ": " << exception_string << std::endl;

                    v8::String::Utf8Value sourceline(message->GetSourceLine());
                    std::cerr << *sourceline << std::endl;

                    int start = message->GetStartColumn();
                    int end = message->GetEndColumn();
                    for (int i = 0; i < start; i++) {
                        std::cerr << " ";
                    }
                    for (int i = start; i < end; i++) {
                        std::cerr << "^";
                    }
                    std::cerr << std::endl;
                }
            }


            v8::Persistent<v8::Object> callbacks_object;
            v8::Persistent<v8::Object> osmium_object;

            struct js_cb {
                v8::Handle<v8::Function> init;
                v8::Handle<v8::Function> before_nodes;
                v8::Handle<v8::Function> node;
                v8::Handle<v8::Function> after_nodes;
                v8::Handle<v8::Function> before_ways;
                v8::Handle<v8::Function> way;
                v8::Handle<v8::Function> after_ways;
                v8::Handle<v8::Function> before_relations;
                v8::Handle<v8::Function> relation;
                v8::Handle<v8::Function> after_relations;
                v8::Handle<v8::Function> area;
                v8::Handle<v8::Function> end;
            } cb;

        public:

            static v8::Handle<v8::Value> Print(const v8::Arguments& args) {
                v8::HandleScope handle_scope;

                for (int i = 0; i < args.Length(); i++) {
                    Osmium::v8_String_to_ostream(args[i]->ToString(), std::cout);
                    std::cout << "\n";
                }

                return handle_scope.Close(v8::Integer::New(1));
            }

            static v8::Handle<v8::Value> Include(const v8::Arguments& args) {
                for (int i=0; i < args.Length(); i++) {
                    v8::String::Utf8Value filename(args[i]);

                    std::string javascript_source = load_file(*filename);

                    if (javascript_source.length() > 0) {
                        v8::TryCatch tryCatch;

                        v8::Handle<v8::Script> script = v8::Script::Compile(v8::String::New(javascript_source.c_str()), v8::String::New(*filename));
                        if (script.IsEmpty()) {
                            std::cerr << "Compiling included script failed:" << std::endl;
                            report_exception(&tryCatch);
                            exit(1);
                        }

                        v8::Handle<v8::Value> result = script->Run();
                        if (result.IsEmpty()) {
                            std::cerr << "Running included script failed:" << std::endl;
                            report_exception(&tryCatch);
                            exit(1);
                        }
                    }
                }
                return v8::Undefined();
            }

            Handler(std::vector<std::string> include_files, const char* filename) :
                Osmium::Handler::Base() {
//                v8::HandleScope handle_scope;
                v8::Handle<v8::String> init_source = v8::String::New("Osmium = { Callbacks: {}, Output: { } };");
                v8::Handle<v8::Script> init_script = v8::Script::Compile(init_source);
                osmium_object = v8::Persistent<v8::Object>::New(init_script->Run()->ToObject());
                v8::Handle<v8::Object> output_object = osmium_object->Get(v8::String::NewSymbol("Output"))->ToObject();

                osmium_object->Set(v8::String::NewSymbol("debug"), v8::Boolean::New(has_debug_level(1)));

                v8::Handle<v8::ObjectTemplate> output_csv_template = v8::ObjectTemplate::New();
                output_csv_template->Set(v8::String::NewSymbol("open"), v8::FunctionTemplate::New(Osmium::Javascript::Wrapper::ExportCSV::open));
                output_object->Set(v8::String::NewSymbol("CSV"), output_csv_template->NewInstance());

                v8::Handle<v8::ObjectTemplate> output_shapefile_template = v8::ObjectTemplate::New();
                output_shapefile_template->Set(v8::String::NewSymbol("open"), v8::FunctionTemplate::New(Osmium::Javascript::Wrapper::ExportShapefile::open));
                output_object->Set(v8::String::NewSymbol("Shapefile"), output_shapefile_template->NewInstance());

                v8::Handle<v8::Object> callbacks_object = osmium_object->Get(v8::String::NewSymbol("Callbacks"))->ToObject();

                v8::TryCatch tryCatch;

                for (std::vector<std::string>::const_iterator vi(include_files.begin()); vi != include_files.end(); ++vi) {
                    if (debug && has_debug_level(1)) {
                        std::cout << "include javascript file: " << *vi << std::endl;
                    }
                    std::string javascript_source = load_file((*vi).c_str());
                    v8::Handle<v8::Script> script = v8::Script::Compile(v8::String::New(javascript_source.c_str()), v8::String::New((*vi).c_str()));
                    if (script.IsEmpty()) {
                        std::cerr << "Compiling script failed:" << std::endl;
                        report_exception(&tryCatch);
                        exit(1);
                    }

                    v8::Handle<v8::Value> result = script->Run();
                    if (result.IsEmpty()) {
                        std::cerr << "Running script failed:" << std::endl;
                        report_exception(&tryCatch);
                        exit(1);
                    }
                }

                std::string javascript_source = load_file(filename);
                if (javascript_source.length() == 0) {
                    std::cerr << "Javascript file " << filename << " is empty" << std::endl;
                    exit(1);
                }

                v8::Handle<v8::Script> script = v8::Script::Compile(v8::String::New(javascript_source.c_str()), v8::String::New(filename));
                if (script.IsEmpty()) {
                    std::cerr << "Compiling script failed:" << std::endl;
                    report_exception(&tryCatch);
                    exit(1);
                }

                v8::Handle<v8::Value> result = script->Run();
                if (result.IsEmpty()) {
                    std::cerr << "Running script failed:" << std::endl;
                    report_exception(&tryCatch);
                    exit(1);
                }

                v8::Handle<v8::Value> cc;

                cc = callbacks_object->Get(v8::String::NewSymbol("init"));
                if (cc->IsFunction()) {
                    cb.init = v8::Handle<v8::Function>::Cast(cc);
                }

                cc = callbacks_object->Get(v8::String::NewSymbol("before_nodes"));
                if (cc->IsFunction()) {
                    cb.before_nodes = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::NewSymbol("node"));
                if (cc->IsFunction()) {
                    cb.node = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::NewSymbol("after_nodes"));
                if (cc->IsFunction()) {
                    cb.after_nodes = v8::Handle<v8::Function>::Cast(cc);
                }

                cc = callbacks_object->Get(v8::String::NewSymbol("before_ways"));
                if (cc->IsFunction()) {
                    cb.before_ways = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::NewSymbol("way"));
                if (cc->IsFunction()) {
                    cb.way = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::NewSymbol("after_ways"));
                if (cc->IsFunction()) {
                    cb.after_ways = v8::Handle<v8::Function>::Cast(cc);
                }

                cc = callbacks_object->Get(v8::String::NewSymbol("before_relations"));
                if (cc->IsFunction()) {
                    cb.before_relations = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::NewSymbol("relation"));
                if (cc->IsFunction()) {
                    cb.relation = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::NewSymbol("after_relations"));
                if (cc->IsFunction()) {
                    cb.after_relations = v8::Handle<v8::Function>::Cast(cc);
                }

                cc = callbacks_object->Get(v8::String::NewSymbol("area"));
                if (cc->IsFunction()) {
                    cb.area = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::NewSymbol("end"));
                if (cc->IsFunction()) {
                    cb.end = v8::Handle<v8::Function>::Cast(cc);
                }
            }

            ~Handler() {
                callbacks_object.Dispose();
            }

            void init(Osmium::OSM::Meta&) {
                if (!cb.init.IsEmpty()) {
                    (void) cb.init->Call(cb.init, 0, 0);
                }
            }

            void before_nodes() {
                if (!cb.before_nodes.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    (void) cb.before_nodes->Call(cb.before_nodes, 0, 0);
                }
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                if (!cb.node.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    v8::Handle<v8::Object> js_object_instance = v8::Local<v8::Object>::New(Osmium::Javascript::Wrapper::OSMNode::get<Osmium::Javascript::Wrapper::OSMNode>().create_instance((void*)(node.get())));
                    (void) cb.node->Call(js_object_instance, 0, 0);
                }
#ifdef OSMIUM_V8_FORCE_GC
                while (!v8::V8::IdleNotification()) { };
#endif // OSMIUM_V8_FORCE_GC
            }

            void after_nodes() {
                if (!cb.after_nodes.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    (void) cb.after_nodes->Call(cb.after_nodes, 0, 0);
                }
            }

            void before_ways() {
                if (!cb.before_ways.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    (void) cb.before_ways->Call(cb.before_ways, 0, 0);
                }
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& way) {
                if (!cb.way.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    v8::Handle<v8::Object> js_object_instance = v8::Local<v8::Object>::New(Osmium::Javascript::Wrapper::OSMWay::get<Osmium::Javascript::Wrapper::OSMWay>().create_instance((void*)(way.get())));
                    (void) cb.way->Call(js_object_instance, 0, 0);
                }
#ifdef OSMIUM_V8_FORCE_GC
                while (!v8::V8::IdleNotification()) { };
#endif // OSMIUM_V8_FORCE_GC
            }

            void after_ways() {
                if (!cb.after_ways.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    (void) cb.after_ways->Call(cb.after_ways, 0, 0);
                }
            }

            void before_relations() {
                if (!cb.before_relations.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    (void) cb.before_relations->Call(cb.before_relations, 0, 0);
                }
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                if (!cb.relation.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    v8::Handle<v8::Object> js_object_instance = v8::Local<v8::Object>::New(Osmium::Javascript::Wrapper::OSMRelation::get<Osmium::Javascript::Wrapper::OSMRelation>().create_instance((void*)(relation.get())));
                    (void) cb.relation->Call(js_object_instance, 0, 0);
                }
#ifdef OSMIUM_V8_FORCE_GC
                while (!v8::V8::IdleNotification()) { };
#endif // OSMIUM_V8_FORCE_GC
            }

            void after_relations() {
                if (!cb.after_relations.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    (void) cb.after_relations->Call(cb.after_relations, 0, 0);
                }
            }

            void area(const shared_ptr<Osmium::OSM::Area const>& area) {
                if (!cb.area.IsEmpty()) {
                    v8::HandleScope handle_scope;
                    v8::Handle<v8::Object> js_object_instance = v8::Local<v8::Object>::New(Osmium::Javascript::Wrapper::OSMArea::get<Osmium::Javascript::Wrapper::OSMArea>().create_instance((void*)(area.get())));
                    (void) cb.area->Call(js_object_instance, 0, 0);
                }
#ifdef OSMIUM_V8_FORCE_GC
                while (!v8::V8::IdleNotification()) { };
#endif // OSMIUM_V8_FORCE_GC
            }

            void final() {
                if (!cb.end.IsEmpty()) {
                    (void) cb.end->Call(cb.end, 0, 0);
                }
            }

        }; // class Handler

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_HANDLER_HPP
