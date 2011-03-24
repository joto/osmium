#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTCSV_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTCSV_HPP

#include "JavascriptOutputCSV.hpp"

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class OutputCSV : public Base {

                static v8::Handle<v8::Value> Print(const v8::Arguments& args) {
                    v8::HandleScope handle_scope;

                    Osmium::Output::CSV *self = (Osmium::Output::CSV *) v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value();

                    for (int i = 0; i < args.Length(); i++) {
                        if (i != 0) {
                            self->out << '\t';
                        }
                        v8_String_to_ostream(args[i]->ToString(), self->out);
                    }
                    self->out << std::endl;
                    return handle_scope.Close(v8::Integer::New(1));
                }

                static v8::Handle<v8::Value> Close(const v8::Arguments& args) {
                    v8::HandleScope handle_scope;
                    Osmium::Output::CSV *self = (Osmium::Output::CSV *) v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value();
                    self->out.flush();
                    self->out.close();
                    return v8::Undefined();
                }

            public:

                OutputCSV() : Base(1) {
                    js_template->Set("print", v8::FunctionTemplate::New(Print));
                    js_template->Set("close", v8::FunctionTemplate::New(Close));
                }

            }; // class OutputCSV

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTCSV_HPP
