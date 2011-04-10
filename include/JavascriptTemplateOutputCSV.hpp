#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTCSV_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTCSV_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class OutputCSV : public Base {

              public:

                OutputCSV() : Base(1) {
                    js_template->Set("print", v8::FunctionTemplate::New(function_template<Osmium::Output::CSV, &Osmium::Output::CSV::js_print>));
                    js_template->Set("close", v8::FunctionTemplate::New(function_template<Osmium::Output::CSV, &Osmium::Output::CSV::js_close>));
                }

            }; // class OutputCSV

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTCSV_HPP
