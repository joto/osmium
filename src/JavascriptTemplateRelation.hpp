#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_RELATION_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_RELATION_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Relation : public Object {

                static v8::Handle<v8::Value> GetMembers(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::Javascript::Relation::Wrapper *self = (Osmium::Javascript::Relation::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return self->js_members_instance;
                }

            public:

                Relation() : Object() {
                    js_template->SetAccessor(v8::String::New("members"), GetMembers);
                }

            }; // class Relation

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_RELATION_HPP
