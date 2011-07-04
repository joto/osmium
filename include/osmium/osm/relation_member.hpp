#ifndef OSMIUM_OSM_RELATION_MEMBER_HPP
#define OSMIUM_OSM_RELATION_MEMBER_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <cstring>
#include <stdexcept>

#include <osmium/osm/types.hpp>

namespace Osmium {

    namespace OSM {

        class RelationMember {

        public:

            static const int max_characters_role = 255;

            static const int max_utf16_length_role = 2 * (max_characters_role + 1); ///< maximum number of UTF-16 units

            static const int max_length_role = 255 * 4 + 1; /* 255 UTF-8 characters + null byte */

            osm_object_id_t ref() const {
                return m_ref;
            }

            RelationMember& ref(osm_object_id_t ref) {
                m_ref = ref;
                return *this;
            }

            char type() const {
                return m_type;
            }

            const char *type_name() const {
                switch (type()) {
                    case 'n':
                        return "node";
                    case 'w':
                        return "way";
                    case 'r':
                        return "relation";
                    default:
                        return "unknown";
                }
            }

            RelationMember& type(char type) {
                m_type = type;
                return *this;
            }

            const char *role() const {
                return m_role;
            }

            RelationMember& role(const char *role) {
                if (! memccpy(m_role, role, 0, max_length_role)) {
                    throw std::length_error("role too long");
                }
                return *this;
            }

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_instance() const {
                return JavascriptTemplate::get<JavascriptTemplate>().create_instance((void *)this);
            }

            v8::Handle<v8::Value> js_ref() const {
                return v8::Number::New(ref());
            }

            v8::Handle<v8::Value> js_type() const {
                char t[2];
                t[0] = type();
                t[1] = 0;
                return v8::String::NewSymbol(t);
            }

            v8::Handle<v8::Value> js_role() const {
                return utf8_to_v8_String<max_utf16_length_role>(role());
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template {

                JavascriptTemplate() : Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("type"), accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_type>);
                    js_template->SetAccessor(v8::String::NewSymbol("ref"),  accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_ref>);
                    js_template->SetAccessor(v8::String::NewSymbol("role"), accessor_getter<Osmium::OSM::RelationMember, &Osmium::OSM::RelationMember::js_role>);
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        private:

            osm_object_id_t m_ref;
            char            m_type;
            char            m_role[max_length_role];

        }; // class RelationMember

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_MEMBER_HPP
