#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_OBJECT_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_OBJECT_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Object : public Base {

                static v8::Handle<v8::Value> GetId(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::OSM::Object *self = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Number::New(self->get_id());
                }

                static v8::Handle<v8::Value> GetVersion(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::OSM::Object *self = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Integer::New(self->version);
                }

                static v8::Handle<v8::Value> GetTimestamp(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::OSM::Object *self = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::String::New(self->get_timestamp_str());
                }

                static v8::Handle<v8::Value> GetUid(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::OSM::Object *self = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Integer::New(self->uid);
                }

                static v8::Handle<v8::Value> GetUser(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::OSM::Object *self = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return utf8_to_v8_String<Osmium::OSM::Object::max_utf16_length_username>(self->user);
                }

                static v8::Handle<v8::Value> GetChangeset(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::OSM::Object *self = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return v8::Number::New(self->changeset);
                }

                static v8::Handle<v8::Value> GetTags(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    Osmium::OSM::Object *self = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    return self->get_tags_instance();
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
