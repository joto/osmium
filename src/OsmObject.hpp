#ifndef OSMIUM_OSM_OBJECT_HPP
#define OSMIUM_OSM_OBJECT_HPP

/** @file
*   @brief Contains the Osmium::OSM::Object class.
*/

#include <stdexcept>
#include <vector>
#include <time.h>

#ifdef WITH_GEOS
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#endif

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

#ifdef WITH_GEOS
            static void init() {
                geos::geom::PrecisionModel *pm = new geos::geom::PrecisionModel();
                global_geometry_factory = new geos::geom::GeometryFactory(pm, -1);
            }

            static geos::geom::GeometryFactory *global_geometry_factory;
#endif

            osm_object_id_t    id;        ///< object id
            osm_version_t      version;   ///< object version
            osm_user_id_t      uid;       ///< user id of user who last changed this object
            osm_changeset_id_t changeset; ///< id of last changeset that changed this object

            // timestamp is stored as string, no need to parse it in most cases
            char timestamp_str[max_length_timestamp];
            time_t timestamp;
            char user[max_length_username]; ///< name of user who last changed this object

            // how many tags are there on this object (XXX we could probably live without this and just use tags.size())
            int num_tags;

        private:

            std::vector<Tag> tags;

#ifdef WITH_GEOS
        protected:
            geos::geom::Geometry *geometry;
#endif

        public:

            void *wrapper;

            public:

            Object() : tags() {
#ifdef WITH_GEOS
                geometry = NULL;
#endif
                reset();
            }

            Object(Object *o) {
                id = o->id;
                version = o->version;
                uid = o->uid;
                changeset = o->changeset;
                strncpy(timestamp_str, o->timestamp_str, max_length_timestamp);
                timestamp = o->timestamp;
                strncpy(user, o->user, max_length_username);
                num_tags = o->num_tags;
                tags = o->tags;
#ifdef WITH_GEOS
                geometry = o->geometry;
#endif
            }

            virtual osm_object_type_t type() const = 0;

            virtual void reset() {
                id               = 0;
                version          = 0;
                uid              = 0;
                changeset        = 0;
                timestamp_str[0] = '\0';
                timestamp        = 0;
                user[0]          = '\0';
                num_tags         = 0;
                tags.clear();
#ifdef WITH_GEOS
                if (geometry) delete geometry; 
                geometry = NULL;
#endif
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

            time_t get_timestamp() {
                if (timestamp == 0) {
                    if (timestamp_str[0] == '\0') {
                        return 0;
                    }
                    struct tm tm;
                    if (strptime(timestamp_str, "%Y-%m-%dT%H:%M:%SZ", &tm) == NULL) {
                        timestamp = -1;
                    } else {
                        timestamp = mktime(&tm);
                    }
                }
                return timestamp;
            }

            const char *get_timestamp_str() {
                if (timestamp_str[0] == '\0') {
                    if (timestamp == 0) {
                        return "";
                    }
                    struct tm *tm;
                    tm = gmtime(&timestamp);
                    strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%dT%H:%M:%SZ", tm);
                }
                return timestamp_str;
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

#ifdef WITH_GEOS
            geos::geom::Geometry *get_geometry() {
                if (!geometry) {
                    build_geometry();
                    if (!geometry) {
                        throw std::runtime_error("can't build geometry");
                    }
                }
                return geometry;
            }

            virtual void build_geometry() {
            }
#endif

        }; // class Object

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_OBJECT_HPP
