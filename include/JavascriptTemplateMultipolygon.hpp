#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_MULTIPOLYGON_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_MULTIPOLYGON_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Multipolygon : public Object {

              public:

                Multipolygon() : Object() {
                    js_template->SetAccessor(v8::String::New("from"), accessor_getter<Osmium::OSM::Multipolygon, &Osmium::OSM::Multipolygon::js_get_from>);
                }

            }; // class Multipolygon

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_MULTIPOLYGON_HPP
