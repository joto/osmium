#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_MULTIPOLYGON_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_MULTIPOLYGON_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Multipolygon : public Object {

                static v8::Handle<v8::Value> GetFrom(v8::Local<v8::String> /*property*/, const v8::AccessorInfo &info) {
                    v8::HandleScope handle_scope;

                    Osmium::Javascript::Multipolygon::Wrapper *self = (Osmium::Javascript::Multipolygon::Wrapper *) v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value();
                    const char *value = (self->object->get_type() == MULTIPOLYGON_FROM_WAY) ? "way" : "relation";
                    return v8::String::New(value);
                }

            public:

                Multipolygon() : Object() {
                    js_template->SetAccessor(v8::String::New("from"), GetFrom);
                }

            }; // class Multipolygon

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_MULTIPOLYGON_HPP
