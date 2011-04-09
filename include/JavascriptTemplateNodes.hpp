#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_NODES_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_NODES_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Nodes : public Base {

              public:

                Nodes() : Base(1) {
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_node_id>,
                        0,
                        0,
                        0,
                        property_enumerator<Osmium::OSM::Way, &Osmium::OSM::Way::js_enumerate_nodes>
                    );
                }

            }; // class Nodes

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_js_template_HPP
