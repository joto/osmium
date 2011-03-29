#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_NODE_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_NODE_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Node : public Object {

                static v8::Handle<v8::Value> GetLon(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Node *self = (Osmium::OSM::Node *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    const char *value = self->get_lon_str();
                    return v8::String::New(value);
                }

                static v8::Handle<v8::Value> GetLat(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Node *self = (Osmium::OSM::Node *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    const char *value = self->get_lat_str();
                    return v8::String::New(value);
                }

                static v8::Handle<v8::Value> GetGeom(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Node *self = (Osmium::OSM::Node *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return self->js_geom_instance;
                }

                public:

                Node() : Object() {
                    js_template->SetAccessor(v8::String::New("lon"),  GetLon);
                    js_template->SetAccessor(v8::String::New("lat"),  GetLat);
                    js_template->SetAccessor(v8::String::New("geom"), GetGeom);
                }

            }; // class Node

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_NODE_HPP
