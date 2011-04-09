#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_NODE_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_NODE_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Node : public Object {

              public:

                Node() : Object() {
                    js_template->SetAccessor(v8::String::New("lon"),  accessor_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_lon>);
                    js_template->SetAccessor(v8::String::New("lat"),  accessor_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_lat>);
                    js_template->SetAccessor(v8::String::New("geom"), accessor_getter<Osmium::OSM::Node, &Osmium::OSM::Node::js_get_geom>);
                }

            }; // class Node

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_NODE_HPP
