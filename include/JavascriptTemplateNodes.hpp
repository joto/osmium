#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_NODES_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_NODES_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Nodes : public Base {

                static v8::Handle<v8::Value> Getter(uint32_t index, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Way *self = (Osmium::OSM::Way *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    osm_object_id_t ref = self->nodes[index];
                    if (sizeof(osm_object_id_t) <= 4)
                        return handle_scope.Close(v8::Integer::New(ref));
                    else
                        return handle_scope.Close(v8::Number::New(ref));
                }

                static v8::Handle<v8::Array> Enumerator(const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Way *self = (Osmium::OSM::Way *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();

                    const osm_sequence_id_t num_nodes = self->node_count();
                    v8::Local<v8::Array> array = v8::Array::New(num_nodes);

                    for (osm_sequence_id_t i=0; i < num_nodes; i++) {
                        v8::Local<v8::Integer> ii = v8::Integer::New(i);
                        array->Set(ii, ii);
                    }

                    return handle_scope.Close(array);
                }

            public:

                Nodes() : Base(1) {
                    js_template->SetIndexedPropertyHandler(Getter, 0, 0, 0, Enumerator);
                }

            }; // class Nodes

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_js_template_HPP
