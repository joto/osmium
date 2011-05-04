#ifndef OSMIUM_HANDLER_JAVASCRIPT_HPP
#define OSMIUM_HANDLER_JAVASCRIPT_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <fstream>

extern v8::Persistent<v8::Context> global_context;

namespace Osmium {

    namespace Handler {

        class Javascript : public Base {

            /***
            * Load Javascript file into string
            */
            static std::string load_file(const char *filename) {
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
                v8::Handle<v8::Function> object;
                v8::Handle<v8::Function> node;
                v8::Handle<v8::Function> way;
                v8::Handle<v8::Function> relation;
                v8::Handle<v8::Function> multipolygon;
                v8::Handle<v8::Function> end;
            } cb;

        public:

            static v8::Handle<v8::Value> Print(const v8::Arguments& args) {
                v8::HandleScope handle_scope;

                for (int i = 0; i < args.Length(); i++) {
                    v8_String_to_ostream(args[i]->ToString(), std::cout);
                    std::cout << std::endl;
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

            static v8::Handle<v8::Value> OutputCSVOpen(const v8::Arguments& args) {
                if (args.Length() != 1) {
                    return v8::Undefined();
                } else {
                    v8::String::Utf8Value str(args[0]);
                    Osmium::Output::CSV *oc = new Osmium::Output::CSV(*str);
                    return oc->get_js_object();
                }
            }

#ifdef OSMIUM_WITH_SHPLIB
            static v8::Handle<v8::Value> OutputShapefileOpen(const v8::Arguments& args) {
                if (args.Length() != 2) {
                    return v8::Undefined();
                } else {
                    v8::String::Utf8Value str(args[0]);
                    v8::String::Utf8Value type(args[1]);
                    Osmium::Output::Shapefile *oc = new Osmium::Output::Shapefile(*str, *type);
                    return oc->get_js_object();
                }
            }
#endif // OSMIUM_WITH_SHPLIB

            Javascript(std::vector<std::string> include_files, const char *filename) : Base() {
//                v8::HandleScope handle_scope;
                v8::Handle<v8::String> init_source = v8::String::New("Osmium = { Callbacks: {}, Output: { } };");
                v8::Handle<v8::Script> init_script = v8::Script::Compile(init_source);
                osmium_object = v8::Persistent<v8::Object>::New(init_script->Run()->ToObject());
                v8::Handle<v8::Object> output_object = osmium_object->Get(v8::String::New("Output"))->ToObject();

                osmium_object->Set(v8::String::New("debug"), v8::Boolean::New(Osmium::global.debug));

                v8::Handle<v8::ObjectTemplate> output_csv_template = v8::ObjectTemplate::New();
                output_csv_template->Set(v8::String::New("open"), v8::FunctionTemplate::New(OutputCSVOpen));
                output_object->Set(v8::String::New("CSV"), output_csv_template->NewInstance());

#ifdef OSMIUM_WITH_SHPLIB
                v8::Handle<v8::ObjectTemplate> output_shapefile_template = v8::ObjectTemplate::New();
                output_shapefile_template->Set(v8::String::New("open"), v8::FunctionTemplate::New(OutputShapefileOpen));
                output_object->Set(v8::String::New("Shapefile"), output_shapefile_template->NewInstance());
#endif // OSMIUM_WITH_SHPLIB

                v8::Handle<v8::Object> callbacks_object = osmium_object->Get(v8::String::New("Callbacks"))->ToObject();

                v8::TryCatch tryCatch;

                for (std::vector<std::string>::iterator vi = include_files.begin(); vi != include_files.end(); vi++) {
                    if (Osmium::global.debug) {
                        std::cerr << "include javascript file: " << *vi << std::endl;
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

                cc = callbacks_object->Get(v8::String::New("init"));
                if (cc->IsFunction()) {
                    cb.init = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::New("object"));
                if (cc->IsFunction()) {
                    cb.object = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::New("node"));
                if (cc->IsFunction()) {
                    cb.node = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::New("way"));
                if (cc->IsFunction()) {
                    cb.way = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::New("relation"));
                if (cc->IsFunction()) {
                    cb.relation = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::New("multipolygon"));
                if (cc->IsFunction()) {
                    cb.multipolygon = v8::Handle<v8::Function>::Cast(cc);
                }
                cc = callbacks_object->Get(v8::String::New("end"));
                if (cc->IsFunction()) {
                    cb.end = v8::Handle<v8::Function>::Cast(cc);
                }
            }

            ~Javascript() {
                callbacks_object.Dispose();
            }

            void callback_init() const {
                if (!cb.init.IsEmpty()) {
                    (void) cb.init->Call(cb.init, 0, 0);
                }
            }

            void callback_object(OSM::Object *object) {
                if (!cb.object.IsEmpty()) {
                    (void) cb.object->Call(object->get_instance(), 0, 0);
                }
#ifdef OSMIUM_V8_FORCE_GC
                while (!v8::V8::IdleNotification()) { };
#endif // OSMIUM_V8_FORCE_GC
            }

            void callback_node(OSM::Node *object) {
                if (!cb.node.IsEmpty()) {
                    (void) cb.node->Call(object->get_instance(), 0, 0);
                }
#ifdef OSMIUM_V8_FORCE_GC
                while (!v8::V8::IdleNotification()) { };
#endif // OSMIUM_V8_FORCE_GC
            }

            void callback_way(OSM::Way *object) {
                if (!cb.way.IsEmpty()) {
                    (void) cb.way->Call(object->get_instance(), 0, 0);
                }
#ifdef OSMIUM_V8_FORCE_GC
                while (!v8::V8::IdleNotification()) { };
#endif // OSMIUM_V8_FORCE_GC
            }

            void callback_relation(OSM::Relation *object) {
                if (!cb.relation.IsEmpty()) {
                    (void) cb.relation->Call(object->get_instance(), 0, 0);
                }
#ifdef OSMIUM_V8_FORCE_GC
                while (!v8::V8::IdleNotification()) { };
#endif // OSMIUM_V8_FORCE_GC
            }

            void callback_multipolygon(OSM::Multipolygon *object) {
                if (!cb.multipolygon.IsEmpty()) {
                    (void) cb.multipolygon->Call(object->get_instance(), 0, 0);
                }
            }

            void callback_final() {
                if (!cb.end.IsEmpty()) {
                    (void) cb.end->Call(cb.end, 0, 0);
                }
            }

        }; // class Javascript

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_JAVASCRIPT_HPP
