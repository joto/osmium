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
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

/** @file
*   @brief Contains the Osmium::OSM::Object class.
*/

#include <cstdlib>
#include <stdexcept>
#include <assert.h>
#include <time.h>

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
#endif // OSMIUM_WITH_SHPLIB

#include <osmium/osm/types.hpp>
#include <osmium/osm/tag_list.hpp>

namespace Osmium {

    namespace OSM {

        /**
        *
        * Parent class for nodes, ways, and relations.
        *
        */
        class Object {

            static const int timestamp_length = 20 + 1; // length of ISO timestamp string yyyy-mm-ddThh:mm:ssZ\0

        public:

            static const int max_characters_username = 255;
            static const int max_utf16_length_username = 2 * (max_characters_username + 1); ///< maximum number of UTF-16 units

            static const int max_length_username = 255 * 4 + 1; ///< maximum length of OSM user name (255 UTF-8 characters + null byte)

        private:

            osm_object_id_t    id;          ///< object id
            osm_version_t      version;     ///< object version
            osm_changeset_id_t changeset;   ///< id of last changeset that changed this object
            time_t             timestamp;   ///< when this object changed last
            osm_user_id_t      uid;         ///< user id of user who last changed this object
            char user[max_length_username]; ///< name of user who last changed this object
            bool               visible;     ///< object visible (only when working with history data)

        public:

            /**
             * Get the object id.
             * @return Object id.
             */
            osm_object_id_t get_id() const {
                return id;
            }

            /**
             * Set the object id.
             * @return Reference to object to make calls chainable.
             */
            Object& set_id(osm_object_id_t value) {
                id = value;
                return *this;
            }

            /**
             * Set the object id.
             * @return Reference to object to make calls chainable.
             */
            Object& set_id(const char *value) {
                id = atol(value);
                return *this;
            }

            /**
             * Get the object version.
             * @return Object version.
             */
            osm_version_t get_version() const {
                return version;
            }

            /**
             * Set the object version.
             * @return Reference to object to make calls chainable.
             */
            Object& set_version(osm_version_t value) {
                version = value;
                return *this;
            }

            /**
             * Set the object version.
             * @return Reference to object to make calls chainable.
             */
            Object& set_version(const char *value) {
                version = atoi(value);
                return *this;
            }

            /**
             * Get the id of the changeset that last changed this object.
             * @return Changeset id.
             */
            osm_changeset_id_t get_changeset() const {
                return changeset;
            }

            /**
             * Set the id of the changeset that last changed this object.
             * @return Reference to object to make calls chainable.
             */
            Object& set_changeset(osm_changeset_id_t value) {
                changeset = value;
                return *this;
            }

            /**
             * Set the id of the changeset that last changed this object.
             * @return Reference to object to make calls chainable.
             */
            Object& set_changeset(const char *value) {
                changeset = atol(value);
                return *this;
            }

            /**
             * Get the id of the user who last changed this object.
             * @return User id.
             */
            osm_user_id_t get_uid() const {
                return uid;
            }

            /**
             * Set the id of the user who last changed this object.
             * @return Reference to object to make calls chainable.
             */
            Object& set_uid(osm_user_id_t value) {
                uid = value;
                return *this;
            }

            /**
             * Set the id of the user who last changed this object.
             * @return Reference to object to make calls chainable.
             */
            Object& set_uid(const char *value) {
                uid = atol(value);
                return *this;
            }

            /**
             * Get the timestamp when this object last changed.
             * @return Timestamp in seconds since epoch.
             */
            time_t get_timestamp() const {
                return timestamp;
            }

            /**
             * The timestamp format for OSM timestamps in strftime(3) format.
             * This is the ISO-Format yyyy-mm-ddThh:mm:ssZ
             */
            static const char *timestamp_format() {
                static const char f[] = "%Y-%m-%dT%H:%M:%SZ";
                return f;
            }

            /**
             * Get the timestamp when this object last changed.
             * @return Timestamp as a string in ISO format (yyyy-mm-ddThh:mm:ssZ).
             */
            std::string get_timestamp_as_string() const {
                struct tm *tm = gmtime(&timestamp);
                std::string s(timestamp_length, '\0');
                /* This const_cast is ok, because we know we have enough space
                   in the string for the format we are using (well at least until
                   the year will have 5 digits). And by setting the size
                   afterwards from the result of strftime we make sure thats set
                   right, too. */
                s.resize(strftime(const_cast<char *>(s.c_str()), timestamp_length, timestamp_format(), tm));
                return s;
            }

            /**
             * Set the timestamp when this object last changed.
             * @param value Time in seconds since epoch.
             * @return Reference to object to make calls chainable.
             */
            Object& set_timestamp(time_t value) {
                timestamp = value;
                return *this;
            }

            /**
             * Set the timestamp when this object last changed.
             * @param value Timestamp in the format "yyyy-mm-ddThh:mm:ssZ".
             * @return Reference to object to make calls chainable.
             * @exception std::invalid_argument Thrown when the given string can't be parsed as a timestamp. The object timestamp will remain unchanged in this case.
             */
            Object& set_timestamp(const char *value) {
                struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                if (strptime(value, timestamp_format(), &tm) == NULL) {
                    throw std::invalid_argument("can't parse timestamp");
                }
                timestamp = timegm(&tm);
                return *this;
            }

            /**
             * Get the name of the user who last changed this object.
             * @return Pointer to internal buffer with user name.
             */
            const char *get_user() const {
                return user;
            }

            /**
             * Set the name of the user who last changed this object.
             * @return Reference to object to make calls chainable.
             * @exception std::length_error Thrown when the username contains more than max_characters_username (255 UTF-8 characters). When the exception is thrown the username is set to "".
             */
            Object& set_user(const char *value) {
                if (! memccpy(user, value, 0, max_length_username)) {
                    user[0] = '\0';
                    throw std::length_error("user name too long");
                }
                return *this;
            }

            /**
             * Get the visible flag of this object.
             * (This is only used in OSM files with history.)
             * @return Visible flag.
             */
            bool get_visible() const {
                return visible;
            }

            /**
             * Set the visible flag of this object.
             * (This is only used in OSM files with history.)
             * @return Reference to object to make calls chainable.
             */
            Object& set_visible(bool value) {
                visible = value;
                return *this;
            }

            /**
             * Set the visible flag of this object.
             * (This is only used in OSM files with history.)
             * @return Reference to object to make calls chainable.
             */
            Object& set_visible(const char *value) {
                if (!strcmp(value, "false")) {
                    visible = false;
                }
                return *this;
            }

        protected:

            Object() : m_tags() {
                reset();
            }

            Object(const Object &o) {
                id        = o.id;
                version   = o.version;
                uid       = o.uid;
                changeset = o.changeset;
                timestamp = o.timestamp;
                m_tags    = o.tags();
                visible   = o.visible;
                strncpy(user, o.user, max_length_username);
            }

            virtual ~Object() {
            }

        public:

            TagList m_tags;

            virtual osm_object_type_t get_type() const = 0;

            virtual void reset() {
                id               = 0;
                version          = 0;
                uid              = 0;
                changeset        = 0;
                timestamp        = 0;
                user[0]          = '\0';
                visible          = true;
                m_tags.clear();
            }

            /**
             * Set named attribute.
             * @param attr Name of the attribute (must be one of "id", "version", "changeset", "timestamp", "uid", "user", "visible")
             * @param value Value of the attribute
             */
            void set_attribute(const char *attr, const char *value) {
                if (!strcmp(attr, "id")) {
                    set_id(value);
                } else if (!strcmp(attr, "version")) {
                    set_version(value);
                } else if (!strcmp(attr, "changeset")) {
                    set_changeset(value);
                } else if (!strcmp(attr, "timestamp")) {
                    set_timestamp(value);
                } else if (!strcmp(attr, "uid")) {
                    set_uid(value);
                } else if (!strcmp(attr, "user")) {
                    set_user(value);
                } else if (!strcmp(attr, "visible")) {
                    set_visible(value);
                }
            }

            const TagList& tags() const {
                return m_tags;
            }

            TagList& tags() {
                return m_tags;
            }

            void add_tag(const char *key, const char *value) {
                m_tags.add(key, value);
            }

            int tag_count() const {
                return m_tags.size();
            }

            const char *get_tag_by_key(const char *key) const {
                return tags().get_tag_by_key(key);
            }

            const char *get_tag_key(int n) const {
                return tags().get_tag_key(n);
            }

            const char *get_tag_value(int n) const {
                return tags().get_tag_value(n);
            }

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_object_instance;

            v8::Local<v8::Object> get_instance() const {
                return js_object_instance;
            }

            v8::Handle<v8::Value> js_get_id() const {
                return v8::Number::New(get_id());
            }

            v8::Handle<v8::Value> js_get_version() const {
                return v8::Integer::New(get_version());
            }

            v8::Handle<v8::Value> js_get_timestamp_as_string() const {
                return v8::String::New(get_timestamp_as_string().c_str());
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
                return tags().js_instance();
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template {

                JavascriptTemplate() : Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("id"),        accessor_getter<Object, &Object::js_get_id>);
                    js_template->SetAccessor(v8::String::NewSymbol("version"),   accessor_getter<Object, &Object::js_get_version>);
                    js_template->SetAccessor(v8::String::NewSymbol("timestamp"), accessor_getter<Object, &Object::js_get_timestamp_as_string>);
                    js_template->SetAccessor(v8::String::NewSymbol("uid"),       accessor_getter<Object, &Object::js_get_uid>);
                    js_template->SetAccessor(v8::String::NewSymbol("user"),      accessor_getter<Object, &Object::js_get_user>);
                    js_template->SetAccessor(v8::String::NewSymbol("changeset"), accessor_getter<Object, &Object::js_get_changeset>);
                    js_template->SetAccessor(v8::String::NewSymbol("tags"),      accessor_getter<Object, &Object::js_get_tags>);
                    js_template->SetAccessor(v8::String::NewSymbol("visible"),   accessor_getter<Object, &Object::js_get_visible>);
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        }; // class Object

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_OBJECT_HPP
