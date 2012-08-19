#ifndef OSMIUM_JAVASCRIPT_WRAPPER_EXPORT_CSV_HPP
#define OSMIUM_JAVASCRIPT_WRAPPER_EXPORT_CSV_HPP

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
#include <osmium/export/csv.hpp>

namespace Osmium {

    namespace Javascript {

        namespace Wrapper {

            struct ExportCSV : public Osmium::Javascript::Template {

                static v8::Handle<v8::Value> open(const v8::Arguments& args) {
                    if (args.Length() != 1) {
                        return v8::Undefined();
                    } else {
                        v8::String::Utf8Value str(args[0]);
                        Osmium::Export::CSV* oc = new Osmium::Export::CSV(*str);
                        return Osmium::Javascript::Wrapper::ExportCSV::get<Osmium::Javascript::Wrapper::ExportCSV>().create_instance((void*)(oc));
                    }
                }

                static v8::Handle<v8::Value> print(const v8::Arguments& args, Osmium::Export::CSV* csv) {
                    for (int i = 0; i < args.Length(); i++) {
                        if (i != 0) {
                            csv->out << '\t';
                        }
                        Osmium::v8_String_to_ostream(args[i]->ToString(), csv->out);
                    }
                    csv->out << std::endl;
                    return v8::Integer::New(1);
                }

                static v8::Handle<v8::Value> close(const v8::Arguments& /*args*/, Osmium::Export::CSV* csv) {
                    csv->out.flush();
                    csv->out.close();
                    return v8::Undefined();
                }

                ExportCSV() :
                    Osmium::Javascript::Template() {
                    js_template->Set("print", v8::FunctionTemplate::New(function_template<Osmium::Export::CSV, print>));
                    js_template->Set("close", v8::FunctionTemplate::New(function_template<Osmium::Export::CSV, close>));
                }

            }; // class ExportCSV

        } // namespace Wrapper

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_WRAPPER_EXPORT_CSV_HPP
