#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_TAGS_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_TAGS_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Tags : public Base {

                static v8::Handle<v8::Value> Getter(v8::Local<v8::String> property, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Object *object = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();

                    const char *key = v8_String_to_utf8<Osmium::OSM::Tag::max_utf16_length_key>(property);
                    const char *value = object->get_tag_by_key(key);
                    if (value) {
                        return handle_scope.Close(utf8_to_v8_String<Osmium::OSM::Tag::max_utf16_length_value>(value));
                    }
                    return v8::Undefined();
                }

                static v8::Handle<v8::Array> Enumerator(const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::OSM::Object *object = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();

                    const int num_tags = object->tag_count();
                    v8::Local<v8::Array> array = v8::Array::New(num_tags);

                    for (int i=0; i < num_tags; i++) {
                        array->Set(v8::Integer::New(i), utf8_to_v8_String<Osmium::OSM::Tag::max_utf16_length_key>(object->get_tag_key(i)));
                    }

                    return handle_scope.Close(array);
                }

            public:

                Tags() : Base(1) {
                    js_template->SetNamedPropertyHandler(Getter, 0, 0, 0, Enumerator);
                }

            }; // class Tags

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_js_template_HPP
