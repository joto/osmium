#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_MEMBER_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_MEMBER_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Member : public Base {

                static v8::Handle<v8::Value> GetType(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    OSM::RelationMember *member = (OSM::RelationMember *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    char type[2];
                    type[0] = member->type;
                    type[1] = 0;
                    return v8::String::New(type);
                }

                static v8::Handle<v8::Value> GetRef(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    OSM::RelationMember *member = (OSM::RelationMember *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Number::New(member->ref);
                }

                static v8::Handle<v8::Value> GetRole(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    OSM::RelationMember *member = (OSM::RelationMember *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::String::New(member->role);
                }

            public:

                Member() : Base(1) {
                    js_template->SetAccessor(v8::String::New("type"), GetType);
                    js_template->SetAccessor(v8::String::New("ref"),  GetRef);
                    js_template->SetAccessor(v8::String::New("role"), GetRole);
                }

            }; // class Member

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_js_template_HPP
