#ifndef OSMIUM_OSM_RELATION_MEMBER_HPP
#define OSMIUM_OSM_RELATION_MEMBER_HPP

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

            osm_object_id_t get_ref() const {
                return ref;
            }

            char get_type() const {
                return type;
            }

            const char *get_role() const {
                return role;
            }

#ifdef WITH_JAVASCRIPT
            v8::Handle<v8::Value> js_get_ref() const {
                return v8::Number::New(get_ref());
            }

            v8::Handle<v8::Value> js_get_type() const {
                char type[2];
                type[0] = get_type();
                type[1] = 0;
                return v8::String::New(type);
            }

            v8::Handle<v8::Value> js_get_role() const {
                return utf8_to_v8_String<max_utf16_length_role>(get_role());
            }
#endif // WITH_JAVASCRIPT

        };

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_RELATION_MEMBER_HPP
