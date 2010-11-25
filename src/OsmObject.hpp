#ifndef OSMIUM_OSM_OBJECT_HPP
#define OSMIUM_OSM_OBJECT_HPP

/** @file
*   @brief Contains the Osmium::OSM::Object class.
*/

#include <stdexcept>
#include <vector>

namespace Osmium {

    namespace OSM {

        /**
        *
        * Parent class for nodes, ways, and relations.
        *
        */
        class Object {

        public:

            static const int max_length_timestamp = 20 + 1; ///< maximum length of OSM object timestamp string (20 characters + null byte)
            static const int max_length_username = 255 * 4 + 1; ///< maximum length of OSM user name (255 UTF-8 characters + null byte)

            osm_object_id_t    id;        ///< object id
            osm_version_t      version;   ///< object version
            osm_user_id_t      uid;       ///< user id of user who last changed this object
            osm_changeset_id_t changeset; ///< id of last changeset that changed this object

            // timestamp is stored as string, no need to parse it in most cases
            char timestamp_str[max_length_timestamp];
            char user[max_length_username]; ///< name of user who last changed this object

            // how many tags are there on this object (XXX we could probably live without this and just use tags.size())
            int num_tags;

        private:

            std::vector<Tag> tags;

        public:

            void *wrapper;

            public:

            Object() : tags() {
                reset();
            }

            virtual osm_object_type_t type() const = 0;

            virtual void reset() {
                id               = 0;
                version          = 0;
                uid              = 0;
                changeset        = 0;
                timestamp_str[0] = 0;
                user[0]          = 0;
                num_tags         = 0;
                tags.clear();
            }

            virtual void set_attribute(const char *attr, const char *value) {
                if (!strcmp(attr, "id")) {
                    id = STR_TO_OBJECT_ID(value);
                } else if (!strcmp(attr, "version")) {
                    version = STR_TO_VERSION(value);
                } else if (!strcmp(attr, "timestamp")) {
                    if (! memccpy(timestamp_str, value, 0, max_length_timestamp)) {
                        throw std::length_error("timestamp too long");
                    }
                } else if (!strcmp(attr, "uid")) {
                    uid = STR_TO_USER_ID(value);
                } else if (!strcmp(attr, "user")) {
                    if (! memccpy(user, value, 0, max_length_username)) {
                        throw std::length_error("user name too long");
                    }
                } else if (!strcmp(attr, "changeset")) {
                    changeset = STR_TO_CHANGESET_ID(value);
                }
            }

            osm_object_id_t get_id() const {
                return id;
            }

            osm_version_t get_version() const {
                return version;
            }

            // get numerical user id, 0 if unknown
            osm_user_id_t get_uid() const {
                return uid;
            }

            osm_changeset_id_t get_changeset() const {
                return changeset;
            }

            void add_tag(const char *key, const char *value) {
                /* first we resize the vector... */
                tags.resize(num_tags+1);
                /* ...so that we can directly write into the memory and avoid
                a second copy */
                if (!memccpy(tags[num_tags].key, key, 0, Tag::max_length_key)) {
                    throw std::length_error("tag key too long");
                }
                if (!memccpy(tags[num_tags].value, value, 0, Tag::max_length_value)) {
                    throw std::length_error("tag value too long");
                }
                num_tags++;
            }

            int tag_count() const {
                return num_tags;
            }

            const char *get_tag_by_key(const char *key) const {
                for (int i=0; i < num_tags; i++) {
                    if (!strcmp(tags[i].key, key)) {
                        return tags[i].value;
                    }
                }
                return 0;
            }

            const char *get_tag_key(int n) const {
                if (n < num_tags) {
                    return tags[n].key;
                }
                throw std::range_error("no tag with this index");
            }

            const char *get_tag_value(int n) const {
                if (n < num_tags) {
                    return tags[n].value;
                }
                throw std::range_error("no tag with this index");
            }

        }; // class Object

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_OBJECT_HPP
