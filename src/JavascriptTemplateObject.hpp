#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_OBJECT_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_OBJECT_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Object : public Base {

                static v8::Handle<v8::Value> GetId(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::Javascript::Object::Wrapper *self = (Osmium::Javascript::Object::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Number::New(self->get_object()->id);
                }

                static v8::Handle<v8::Value> GetVersion(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::Javascript::Object::Wrapper *self = (Osmium::Javascript::Object::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Integer::New(self->get_object()->version);
                }

                static v8::Handle<v8::Value> GetTimestamp(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::Javascript::Object::Wrapper *self = (Osmium::Javascript::Object::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::String::New(self->get_object()->timestamp_str);
                }

                static v8::Handle<v8::Value> GetUid(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::Javascript::Object::Wrapper *self = (Osmium::Javascript::Object::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Integer::New(self->get_object()->uid);
                }

                static v8::Handle<v8::Value> GetUser(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::Javascript::Object::Wrapper *self = (Osmium::Javascript::Object::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::String::New(self->get_object()->user);
                }

                static v8::Handle<v8::Value> GetChangeset(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::Javascript::Object::Wrapper *self = (Osmium::Javascript::Object::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Number::New(self->get_object()->changeset);
                }

                static v8::Handle<v8::Value> GetTags(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::Javascript::Object::Wrapper *self = (Osmium::Javascript::Object::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return self->js_tags_instance;
                }

            protected:

                Object() : Base(1) {
                    js_template->SetAccessor(v8::String::New("id"),        GetId);
                    js_template->SetAccessor(v8::String::New("version"),   GetVersion);
                    js_template->SetAccessor(v8::String::New("timestamp"), GetTimestamp);
                    js_template->SetAccessor(v8::String::New("uid"),       GetUid);
                    js_template->SetAccessor(v8::String::New("user"),      GetUser);
                    js_template->SetAccessor(v8::String::New("changeset"), GetChangeset);
                    js_template->SetAccessor(v8::String::New("tags"),      GetTags);
                }

            }; // class Object

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_OBJECT_HPP
