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
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <vector>
#include <sstream>

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
#endif // OSMIUM_WITH_SHPLIB

namespace Osmium {

    namespace OSM {

        class Relation : public Object {

            // how many members are there on this object
            osm_sequence_id_t num_members;

            std::vector<RelationMember> members;

        public:

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_members_instance;
#endif // OSMIUM_WITH_JAVASCRIPT

            Relation() : Object(), members() {
                reset();
#ifdef OSMIUM_WITH_JAVASCRIPT
                js_tags_instance    = Osmium::Javascript::Template::create_tags_instance(this);
                js_object_instance  = Osmium::Javascript::Template::create_relation_instance(this);
                js_members_instance = Osmium::Javascript::Template::create_relation_members_instance(this);
#endif // OSMIUM_WITH_JAVASCRIPT
            }

            Relation(const Relation &r) : Object(r) {
                num_members = r.num_members;
                members = r.members;
#ifdef OSMIUM_WITH_JAVASCRIPT
                js_tags_instance    = Osmium::Javascript::Template::create_tags_instance(this);
                js_object_instance  = Osmium::Javascript::Template::create_relation_instance(this);
                js_members_instance = Osmium::Javascript::Template::create_relation_members_instance(this);
#endif // OSMIUM_WITH_JAVASCRIPT
            }

            osm_object_type_t get_type() const {
                return RELATION;
            }

            void reset() {
                Object::reset();
                num_members = 0;
                members.clear();
            }

            void add_member(const char type, osm_object_id_t ref, const char *role) {
                /* first we resize the vector... */
                members.resize(num_members+1);
                /* ...and get an address for the new element... */
                RelationMember *m = &members[num_members++];
                /* ...so that we can directly write into the memory and avoid
                a second copy */
                m->type = type;
                m->ref = ref;
                if (! memccpy(m->role, role, 0, RelationMember::max_length_role)) {
                    throw std::length_error("role too long");
                }
            }

            osm_sequence_id_t member_count() const {
                return num_members;
            }

            const RelationMember *get_member(osm_sequence_id_t index) const {
                if (index < num_members) {
                    return &members[index];
                }
                return NULL;
            }

#ifdef OSMIUM_WITH_SHPLIB
            SHPObject *create_shpobject(int /*shp_type*/) {
                throw std::runtime_error("a relation can not be added to a shapefile of any type");
            }
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Handle<v8::Value> js_get_members() const {
                return js_members_instance;
            }

            v8::Handle<v8::Value> js_get_member(uint32_t index) {
                v8::Local<v8::Object> member = Osmium::Javascript::Template::create_relation_member_instance(this);
                member->SetInternalField(0, v8::External::New((void *)get_member(index)));

                return member;
            }

            v8::Handle<v8::Array> js_enumerate_members() const {
                v8::Local<v8::Array> array = v8::Array::New(num_members);

                for (osm_sequence_id_t i=0; i < num_members; i++) {
                    v8::Local<v8::Integer> ii = v8::Integer::New(i);
                    array->Set(ii, ii);
                }

                return array;
            }

#endif // OSMIUM_WITH_JAVASCRIPT

        }; // class Relation

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_HPP
