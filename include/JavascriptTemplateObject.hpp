#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_OBJECT_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_OBJECT_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Object : public Base {

              protected:

                Object() : Base(1) {
                    js_template->SetAccessor(v8::String::New("id"),        accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_id>);
                    js_template->SetAccessor(v8::String::New("version"),   accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_version>);
                    js_template->SetAccessor(v8::String::New("timestamp"), accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_timestamp_str>);
                    js_template->SetAccessor(v8::String::New("uid"),       accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_uid>);
                    js_template->SetAccessor(v8::String::New("user"),      accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_user>);
                    js_template->SetAccessor(v8::String::New("changeset"), accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_changeset>);
                    js_template->SetAccessor(v8::String::New("tags"),      accessor_getter<Osmium::OSM::Object, &Osmium::OSM::Object::js_get_tags>);
                }

            }; // class Object

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_OBJECT_HPP
