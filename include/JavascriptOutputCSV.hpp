#ifndef OSMIUM_OUTPUT_CSV_HPP
#define OSMIUM_OUTPUT_CSV_HPP

#include <fstream>

#include "Javascript.hpp"

namespace Osmium {

    namespace Output {

        class CSV {

            public:

            std::ofstream out;

            v8::Persistent<v8::Object> js_object;

            public:

            static void JS_Cleanup(v8::Persistent<v8::Value> /*obj*/, void *self) { // XXX is never called
                printf("cleanup\n\n");
                ((CSV *)self)->out.flush();
                ((CSV *)self)->out.close();
            }

            CSV(const char *filename) {
                out.open(filename);
                js_object = v8::Persistent<v8::Object>::New( Osmium::Javascript::Template::create_output_csv_instance(this) );
                // js_object.MakeWeak((void *)(this), JS_Cleanup); // XXX doesn't work!?
            }

            ~CSV() {
                out.flush();
                out.close();
            }

            v8::Handle<v8::Object> get_js_object() {
                return js_object;
            }

        }; // class CSV

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_CSV_HPP
