#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_RELATION_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_RELATION_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            class Relation : public Object {

              public:

                Relation() : Object() {
                    js_template->SetAccessor(v8::String::New("members"), accessor_getter<Osmium::OSM::Relation, &Osmium::OSM::Relation::js_get_members>);
                }

            }; // class Relation

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_RELATION_HPP
