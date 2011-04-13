#ifndef OSMIUM_OUTPUT_CSV_HPP
#define OSMIUM_OUTPUT_CSV_HPP

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

namespace Osmium {

    namespace Output {

        class CSV {

        public:

            std::ofstream out;

            v8::Persistent<v8::Object> js_object;

            CSV(const char *filename) {
                out.open(filename);
                js_object = v8::Persistent<v8::Object>::New( Osmium::Javascript::Template::create_output_csv_instance(this) );
            }

            ~CSV() {
                out.flush();
                out.close();
            }

            v8::Handle<v8::Object> get_js_object() const {
                return js_object;
            }

            v8::Handle<v8::Value> js_print(const v8::Arguments& args) {
                for (int i = 0; i < args.Length(); i++) {
                    if (i != 0) {
                        out << '\t';
                    }
                    v8_String_to_ostream(args[i]->ToString(), out);
                }
                out << std::endl;
                return v8::Integer::New(1);
            }

            v8::Handle<v8::Value> js_close(const v8::Arguments& /*args*/) {
                out.flush();
                out.close();
                return v8::Undefined();
            }

        }; // class CSV

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_CSV_HPP
