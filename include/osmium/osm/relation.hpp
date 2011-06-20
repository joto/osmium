#ifndef OSMIUM_OSM_RELATION_HPP
#define OSMIUM_OSM_RELATION_HPP

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

namespace Osmium {

    namespace OSM {

        class Relation : public Object {

        public:

            Relation() : Object(), m_members() {
                reset();
#ifdef OSMIUM_WITH_JAVASCRIPT
                js_object_instance  = JavascriptTemplate::get<JavascriptTemplate>().create_instance(this);
#endif // OSMIUM_WITH_JAVASCRIPT
            }

            Relation(const Relation &r) : Object(r) {
                m_members = r.members();
#ifdef OSMIUM_WITH_JAVASCRIPT
                js_object_instance  = JavascriptTemplate::get<JavascriptTemplate>().create_instance(this);
#endif // OSMIUM_WITH_JAVASCRIPT
            }

            const RelationMemberList& members() const {
                return m_members;
            }

            osm_object_type_t get_type() const {
                return RELATION;
            }

            void reset() {
                Object::reset();
                m_members.clear();
            }

            void add_member(const char type, osm_object_id_t ref, const char *role) {
                m_members.add_member(type, ref, role);
            }

            const RelationMember *get_member(osm_sequence_id_t index) const {
                if (index < m_members.size()) {
                    return &m_members[index];
                }
                return NULL;
            }

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Handle<v8::Value> js_members() const {
                return members().js_instance();
            }

            struct JavascriptTemplate : public Osmium::OSM::Object::JavascriptTemplate {

                JavascriptTemplate() : Osmium::OSM::Object::JavascriptTemplate() {
                    js_template->SetAccessor(v8::String::New("members"), accessor_getter<Relation, &Relation::js_members>);
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        private:

            RelationMemberList m_members;

        }; // class Relation

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_HPP
