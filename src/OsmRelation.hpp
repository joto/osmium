#ifndef OSMIUM_OSM_RELATION_HPP
#define OSMIUM_OSM_RELATION_HPP

#include <vector>
#include <sstream>

namespace Osmium {

    namespace OSM {

        class RelationMember {

        public:
            static const int max_length_role = 255 * 4 + 1; /* 255 UTF-8 characters + null byte */

            osm_object_id_t ref;
            char            type;
            char            role[max_length_role];

        };

        class Relation : public Object {

            // how many members are there on this object
            osm_sequence_id_t num_members;

            std::vector<RelationMember> members;

        public:

            Relation() : Object(), members() {
            }

            osm_object_type_t type() const {
                return RELATION;
            }

            void reset() {
                num_members = 0;
                members.clear();
                Object::reset();
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

            RelationMember *get_member(int index) {
                if (index < num_members) {
                    return &members[index];
                }
                return 0;
            }

        }; // class Relation

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_HPP
