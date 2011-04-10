#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTSHAPEFILE_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTSHAPEFILE_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

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

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_OUTPUTSHAPEFILE_HPP
