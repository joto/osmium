#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_TAGS_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_TAGS_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Tags : public Base {

              public:

                Tags() : Base(1) {
                    js_template->SetNamedPropertyHandler(
                        named_property_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_tag_value_by_key>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Object, &Osmium::OSM::Object::js_enumerate_tag_keys>
                    );
                }

            }; // class Tags

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_js_template_HPP
