
#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_NODEGEOM_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_NODEGEOM_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class NodeGeom : public Base {

                static v8::Handle<v8::Value> Getter(v8::Local<v8::String> property, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::Javascript::Node::Wrapper *self = (Osmium::Javascript::Node::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    v8::String::Utf8Value key(property);
                    std::ostringstream oss;

                    if (!strcmp(*key, "as_wkt")) {
                        oss << "POINT(" << self->object->get_lon_str() << " " << self->object->get_lat_str() << ")";
                    } else if (!strcmp(*key, "as_ewkt")) {
                        oss << "SRID=4326;POINT(" << self->object->get_lon_str() << " " << self->object->get_lat_str() << ")";
                    } else if (!strcmp(*key, "as_hex_wkb")) {
                        oss << self->object->geom_as_hex_wkb();
        //            } else if (!strcmp(*key, "as_hex_ewkb")) {
        //                oss << self->object->geom.to_hex();             TODO TODO
                    }

                    return v8::String::New(oss.str().c_str());
                }

            public:

                NodeGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(Getter);
                }

            }; // class NodeGeom

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_NODEGEOM_HPP
