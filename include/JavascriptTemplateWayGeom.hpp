#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_WAYGEOM_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_WAYGEOM_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class WayGeom : public Base {

                static v8::Handle<v8::Value> Getter(v8::Local<v8::String> property, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::Javascript::Way::Wrapper *self = (Osmium::Javascript::Way::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    v8::String::Utf8Value key(property);

                    if (!strcmp(*key, "linestring_wkt")) {
                        const osm_sequence_id_t num_nodes = self->object->node_count();
                        std::ostringstream oss;
                        oss << "LINESTRING(";
                        for (osm_sequence_id_t i=0; i < num_nodes; i++) {
                            if (i != 0) {
                                oss << ",";
                            }
                            oss << self->object->lon[i] << " " << self->object->lat[i];
                        }
                        oss << ")";
                        return v8::String::New(oss.str().c_str());
                    }

                    return v8::Undefined();
                }

                static v8::Handle<v8::Array> Enumerator(const v8::AccessorInfo &/*info*/) {
                    v8::HandleScope handle_scope;
                    v8::Local<v8::Array> array = v8::Array::New(1);

                    array->Set(v8::Integer::New(0), v8::String::New("linestring_wkt"));

                    return handle_scope.Close(array);
                }

            public:

                WayGeom() : Base(1) {
                    js_template->SetNamedPropertyHandler(Getter, 0, 0, 0, Enumerator);
                }

            }; // class WayGeom

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_WAYGEOM_HPP
