#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_WAY_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_WAY_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Way : public Object {

              public:

                Way() : Object() {
                    js_template->SetAccessor(v8::String::New("nodes"), accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_nodes>);
                    js_template->SetAccessor(v8::String::New("geom"),  accessor_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_geom>);
                }

            }; // class Way

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_WAY_HPP
