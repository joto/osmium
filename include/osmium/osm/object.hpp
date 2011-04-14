#ifndef OSMIUM_OSM_OBJECT_HPP
#define OSMIUM_OSM_OBJECT_HPP

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

/** @file
*   @brief Contains the Osmium::OSM::Object class.
*/

#include <stdexcept>
#include <vector>
#include <cstring>
#include <assert.h>
#include <time.h>

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
#endif // OSMIUM_WITH_SHPLIB

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

            static const int max_characters_username = 255;
            static const int max_utf16_length_username = 2 * (max_characters_username + 1); ///< maximum number of UTF-16 units

            static const int max_length_username = 255 * 4 + 1; ///< maximum length of OSM user name (255 UTF-8 characters + null byte)

            osm_object_id_t    id;        ///< object id
            osm_version_t      version;   ///< object version
            osm_user_id_t      uid;       ///< user id of user who last changed this object
            osm_changeset_id_t changeset; ///< id of last changeset that changed this object

            time_t timestamp;
            char user[max_length_username]; ///< name of user who last changed this object

            bool visible;

        private:

            static osm_object_id_t string_to_osm_object_id(const char *x) {
                return atol(x);
            }

            static osm_version_t string_to_osm_version(const char *x) {
                return atoi(x);
            }

            static osm_changeset_id_t string_to_osm_changeset_id(const char *x) {
                return atol(x);
            }

            static osm_user_id_t string_to_osm_user_id(const char *x) {
                return atol(x);
            }

        protected:

            // how many tags are there on this object (XXX we could probably live without this and just use tags.size())
            int num_tags;

            Object() : tags() {
                reset();
            }

            Object(const Object &o) {
                id        = o.id;
                version   = o.version;
                uid       = o.uid;
                changeset = o.changeset;
                timestamp = o.timestamp;
                num_tags  = o.num_tags;
                tags      = o.tags;
                visible   = o.visible;
                strncpy(user, o.user, max_length_username);
            }

            ~Object() {
            }

        public:

            std::vector<Tag> tags;

            virtual osm_object_type_t get_type() const = 0;

            virtual void reset() {
                id               = 0;
                version          = 0;
                uid              = 0;
                changeset        = 0;
                timestamp        = 0;
                user[0]          = '\0';
                num_tags         = 0;
                tags.clear();
                visible          = true;
            }

            void set_attribute(const char *attr, const char *value) {
                if (!strcmp(attr, "id")) {
                    id = string_to_osm_object_id(value);
                } else if (!strcmp(attr, "version")) {
                    version = string_to_osm_version(value);
                } else if (!strcmp(attr, "timestamp")) {
                    struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                    if (strptime(value, "%Y-%m-%dT%H:%M:%SZ", &tm) == NULL) {
                        throw std::length_error("can't parse timestamp");
                    } else {
                        timestamp = timegm(&tm);
                    }
                } else if (!strcmp(attr, "uid")) {
                    uid = string_to_osm_user_id(value);
                } else if (!strcmp(attr, "user")) {
                    if (! memccpy(user, value, 0, max_length_username)) {
                        throw std::length_error("user name too long");
                    }
                } else if (!strcmp(attr, "changeset")) {
                    changeset = string_to_osm_changeset_id(value);
                } else if (!strcmp(attr, "visible")) {
                    if (!strcmp(value, "false")) {
                        visible = false;
                    }
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

            const char *get_user() const {
                return user;
            }

            osm_changeset_id_t get_changeset() const {
                return changeset;
            }

            time_t get_timestamp() const {
                return timestamp;
            }

            /**
             * Return timestamp as string.
             * @returns Pointer to \0-terminated string in a static buffer.
             */
            const char *get_timestamp_str() const {
                static char timestamp_str[max_length_timestamp+1];
                struct tm *tm = gmtime(&timestamp);
                strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%dT%H:%M:%SZ", tm);
                return timestamp_str;
            }

            /**
             * Return visible flag. This is only used in OSM files with history.
             * @returns Visible flag.
             */
            bool get_visible() const {
                return visible;
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

#ifdef OSMIUM_WITH_SHPLIB
            virtual SHPObject *create_shpobject(int shp_type) = 0;
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_object_instance;
            v8::Local<v8::Object> js_tags_instance;
#endif // OSMIUM_WITH_JAVASCRIPT

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> get_instance() const {
                return js_object_instance;
            }

            v8::Handle<v8::Value> js_get_id() const {
                return v8::Number::New(get_id());
            }

            v8::Handle<v8::Value> js_get_version() const {
                return v8::Integer::New(get_version());
            }

            v8::Handle<v8::Value> js_get_timestamp_str() const {
                return v8::String::New(get_timestamp_str());
            }

            v8::Handle<v8::Value> js_get_uid() const {
                return v8::Integer::New(get_uid());
            }

            v8::Handle<v8::Value> js_get_user() const {
                return utf8_to_v8_String<max_utf16_length_username>(get_user());
            }

            v8::Handle<v8::Value> js_get_changeset() const {
                return v8::Number::New(get_changeset());
            }

            v8::Handle<v8::Value> js_get_visible() const {
                return v8::Boolean::New(get_visible());
            }

            v8::Handle<v8::Value> js_get_tags() const {
                return js_tags_instance;
            }

            v8::Handle<v8::Value> js_get_tag_value_by_key(v8::Local<v8::String> property) const {
                const char *key = v8_String_to_utf8<Osmium::OSM::Tag::max_utf16_length_key>(property);
                const char *value = get_tag_by_key(key);
                if (value) {
                    return utf8_to_v8_String<Osmium::OSM::Tag::max_utf16_length_value>(value);
                }
                return v8::Undefined();
            }

            v8::Handle<v8::Array> js_enumerate_tag_keys() const {
                v8::Local<v8::Array> array = v8::Array::New(num_tags);

                for (int i=0; i < num_tags; i++) {
                    array->Set(v8::Integer::New(i), utf8_to_v8_String<Osmium::OSM::Tag::max_utf16_length_key>(get_tag_key(i)));
                }

                return array;
            }

#endif // OSMIUM_WITH_JAVASCRIPT

        }; // class Object

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_OBJECT_HPP
