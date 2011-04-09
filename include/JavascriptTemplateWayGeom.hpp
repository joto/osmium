#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_WAYGEOM_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_WAYGEOM_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class WayGeom : public Base {

              public:

                WayGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(named_property_getter<Osmium::OSM::Way, &Osmium::OSM::Way::js_get_geom_property>);
                }

            }; // class WayGeom

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_WAYGEOM_HPP
