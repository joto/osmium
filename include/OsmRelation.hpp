#ifndef OSMIUM_OSM_RELATION_HPP
#define OSMIUM_OSM_RELATION_HPP

#include <vector>
#include <sstream>

#ifdef WITH_SHPLIB
#include <shapefil.h>
#endif

namespace Osmium {

    namespace OSM {

        class RelationMember {

          public:

            static const int max_characters_role = 255;

            static const int max_utf16_length_role = 2 * (max_characters_role + 1); ///< maximum number of UTF-16 units

            static const int max_length_role = 255 * 4 + 1; /* 255 UTF-8 characters + null byte */

            osm_object_id_t ref;
            char            type;
            char            role[max_length_role];
            Object         *object;

        };

        class Relation : public Object {

            // how many members are there on this object
            osm_sequence_id_t num_members;

            std::vector<RelationMember> members;

          public:

            Relation() : Object(), members() {
            }

            Relation(const Relation &r) : Object(r) {
                num_members = r.num_members;
                members = r.members;
            }

            osm_object_type_t get_type() const {
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

#ifdef WITH_SHPLIB
            SHPObject *create_shpobject(int /*shp_type*/) {
                throw std::runtime_error("a relation can not be added to a shapefile of any type");
            }
#endif

        }; // class Relation

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_HPP
