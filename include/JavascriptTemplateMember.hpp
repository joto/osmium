#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_MEMBER_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_MEMBER_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Member : public Base {

              public:

                Member() : Base(1) {
                    js_template->SetAccessor(v8::String::New("type"), accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_get_type>);
                    js_template->SetAccessor(v8::String::New("ref"),  accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_get_ref>);
                    js_template->SetAccessor(v8::String::New("role"), accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_get_role>);
                }

            }; // class Member

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_js_template_HPP
