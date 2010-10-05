#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_MEMBERS_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_MEMBERS_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Members : public Base {

                static v8::Handle<v8::Value> Getter(uint32_t index, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::Javascript::Relation::Wrapper *self = (Osmium::Javascript::Relation::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    v8::Local<v8::Object> member = create_relation_member_instance(self);
                    member->SetInternalField(0, v8::External::New(self->object->get_member(index)));

                    return handle_scope.Close(member);
                }

                static v8::Handle<v8::Array> Enumerator(const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::Javascript::Relation::Wrapper *self = (Osmium::Javascript::Relation::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();

                    const osm_sequence_id_t num_members = self->object->member_count();
                    v8::Local<v8::Array> array = v8::Array::New(num_members);

                    for (osm_sequence_id_t i=0; i < num_members; i++) {
                        v8::Local<v8::Integer> ii = v8::Integer::New(i);
                        array->Set(ii, ii);
                    }

                    return handle_scope.Close(array);
                }

            public:

                Members() : Base(1) {
                    js_template->SetIndexedPropertyHandler(Getter, 0, 0, 0, Enumerator);
                }

            }; // class Members

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_js_template_HPP
