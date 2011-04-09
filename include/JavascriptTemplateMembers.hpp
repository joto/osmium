#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_MEMBERS_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_MEMBERS_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Members : public Base {

              public:

                Members() : Base(1) {
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_get_member>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_enumerate_members>
                    );
                }

            }; // class Members

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_js_template_HPP
