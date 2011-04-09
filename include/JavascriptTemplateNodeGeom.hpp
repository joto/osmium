#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_NODEGEOM_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_NODEGEOM_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class NodeGeom : public Base {

              public:

                NodeGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(named_property_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_geom_property>);
                }

            }; // class NodeGeom

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_NODEGEOM_HPP
