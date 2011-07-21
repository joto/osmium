#ifndef OSMIUM_OSM_RELATION_MEMBER_LIST_HPP
#define OSMIUM_OSM_RELATION_MEMBER_LIST_HPP

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

#include <vector>

#include <osmium/osm/relation_member.hpp>

namespace Osmium {

    namespace OSM {

        class RelationMemberList {

        public:

            RelationMemberList() : m_list() {
            }

            osm_sequence_id_t size() const {
                return m_list.size();
            }

            void clear() {
                m_list.clear();
            }

            RelationMember& operator[](int i) {
                return m_list[i];
            }

            const RelationMember& operator[](int i) const {
                return m_list[i];
            }

            typedef std::vector<RelationMember>::iterator iterator;
            typedef std::vector<RelationMember>::const_iterator const_iterator;

            iterator begin() {
                return m_list.begin();
            }

            const_iterator begin() const {
                return m_list.begin();
            }

            iterator end() {
                return m_list.end();
            }

            const_iterator end() const {
                return m_list.end();
            }

            void add_member(const char type, osm_object_id_t ref, const char *role) {
                /* first we resize the vector... */
                m_list.resize(m_list.size()+1);
                /* ...and get an address for the new element... */
                RelationMember *m = &m_list[m_list.size()-1];
                /* ...so that we can directly write into the memory and avoid
                a second copy */
                m->type(type);
                m->ref(ref);
                m->role(role);
            }

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_instance() const {
                return JavascriptTemplate::get<JavascriptTemplate>().create_instance((void *)this);
            }

            v8::Handle<v8::Value> js_get_member(uint32_t index) {
                return m_list[index].js_instance();
            }

            v8::Handle<v8::Array> js_enumerate_members() const {
                v8::HandleScope scope;
                v8::Local<v8::Array> array = v8::Array::New(m_list.size());

                for (unsigned int i=0; i < m_list.size(); i++) {
                    array->Set(i, v8::Integer::New(i));
                }

                return scope.Close(array);
            }

            v8::Handle<v8::Value> js_length() const {
                return v8::Number::New(m_list.size());
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template {

                JavascriptTemplate() : Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("length"), accessor_getter<RelationMemberList, &RelationMemberList::js_length>);
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<RelationMemberList, &RelationMemberList::js_get_member>,
                        0,
                        0,
                        0,
                        property_enumerator<RelationMemberList, &RelationMemberList::js_enumerate_members>
                    );
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        private:

            std::vector<RelationMember> m_list;

        }; // class RelationMemberList

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_MEMBER_LIST_HPP
