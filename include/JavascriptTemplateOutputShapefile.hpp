#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTSHAPEFILE_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTSHAPEFILE_HPP

#include "JavascriptOutputShapefile.hpp"

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class OutputShapefile : public Base {

                static v8::Handle<v8::Value> AddField(const v8::Arguments& args) {
                    v8::HandleScope handle_scope;

                    Osmium::Output::Shapefile *self = (Osmium::Output::Shapefile *) v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value();

                    if (args.Length() < 3 || args.Length() > 4) {
                        throw std::runtime_error("wrong number of arguments");
                    }
                    v8::String::Utf8Value name(args[0]);
                    v8::String::Utf8Value type(args[1]);
                    int width = args[2]->Int32Value();
                    int decimals = (args.Length() == 4) ? args[3]->Int32Value() : 0;
                    self->add_field(*name, *type, width, decimals);

                    return handle_scope.Close(v8::Integer::New(1));
                }

                static v8::Handle<v8::Value> Add(const v8::Arguments& args) {
                    v8::HandleScope handle_scope;

                    Osmium::Output::Shapefile *self = (Osmium::Output::Shapefile *) v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value();

                    if (args.Length() != 2) {
                        throw std::runtime_error("wrong number of arguments");
                    }

                    v8::Local<v8::Object> xxx = v8::Local<v8::Object>::Cast(args[0]);
                    Osmium::OSM::Object *object = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(xxx->GetInternalField(0))->Value();

                    self->add(object, v8::Local<v8::Object>::Cast(args[1]));

                    return handle_scope.Close(v8::Integer::New(1));
                }

                static v8::Handle<v8::Value> Close(const v8::Arguments& args) {
                    v8::HandleScope handle_scope;
                    Osmium::Output::Shapefile *self = (Osmium::Output::Shapefile *) v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value();
                    self->close();
                    return v8::Undefined();
                }

            public:

                OutputShapefile() : Base(1) {
                    js_template->Set("add_field", v8::FunctionTemplate::New(AddField));
                    js_template->Set("add", v8::FunctionTemplate::New(Add));
                    js_template->Set("close", v8::FunctionTemplate::New(Close));
                }

            }; // class OutputShapefile

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTSHAPEFILE_HPP
