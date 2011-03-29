#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_WAY_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_WAY_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Way : public Object {

                static v8::Handle<v8::Value> GetNodes(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Way *self = (Osmium::OSM::Way *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return self->js_nodes_instance;
                }

                static v8::Handle<v8::Value> GetGeom(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Way *self = (Osmium::OSM::Way *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return self->js_geom_instance;
                }

            public:

                Way() : Object() {
                    js_template->SetAccessor(v8::String::New("nodes"), GetNodes);
                    js_template->SetAccessor(v8::String::New("geom"),  GetGeom);
                }

            }; // class Way

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_WAY_HPP
