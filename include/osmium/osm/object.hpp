#ifndef OSMIUM_OSM_OBJECT_HPP
#define OSMIUM_OSM_OBJECT_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <string>

#include <osmium/smart_ptr.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/tag_list.hpp>
#include <osmium/utils/timestamp.hpp>

namespace Osmium {

    namespace OSM {

        /**
        * Parent class for nodes, ways, and relations.
        */
        class Object {

        public:

            static const unsigned int max_characters_username = 255;
            static const unsigned int max_utf16_length_username = 2 * (max_characters_username + 1); ///< maximum number of UTF-16 units

            static const unsigned int max_length_username = 255 * 4 + 1; ///< maximum length of OSM user name (255 UTF-8 characters + null byte)

            osm_object_id_t id() const {
                return m_id;
            }

            Object& id(osm_object_id_t id) {
                m_id = id;
                return *this;
            }

            Object& id(const char* id) {
                m_id = Osmium::string_to_osm_object_id_t(id);
                return *this;
            }

            osm_version_t version() const {
                return m_version;
            }

            Object& version(osm_version_t version) {
                m_version = version;
                return *this;
            }

            Object& version(const char* version) {
                m_version = Osmium::string_to_osm_version_t(version);
                return *this;
            }

            osm_changeset_id_t changeset() const {
                return m_changeset;
            }

            Object& changeset(osm_changeset_id_t changeset) {
                m_changeset = changeset;
                return *this;
            }

            Object& changeset(const char* changeset) {
                m_changeset = Osmium::string_to_osm_changeset_id_t(changeset);
                return *this;
            }

            osm_user_id_t uid() const {
                return m_uid;
            }

            Object& uid(osm_user_id_t uid) {
                m_uid = uid;
                return *this;
            }

            Object& uid(const char* uid) {
                m_uid = Osmium::string_to_osm_user_id_t(uid);
                return *this;
            }

            bool user_is_anonymous() const {
                return m_uid == -1;
            }

            time_t timestamp() const {
                return m_timestamp;
            }

            time_t endtime() const {
                return m_endtime;
            }

            /**
             * Get the timestamp when this object last changed.
             * @return Timestamp as a string in ISO format (yyyy-mm-ddThh:mm:ssZ). Empty string if unset.
             */
            const std::string timestamp_as_string() const {
                return Osmium::Timestamp::to_iso(m_timestamp);
            }

            /**
             * Get the timestamp until which this object is valid.
             * @return Timestamp as a string in ISO format (yyyy-mm-ddThh:mm:ssZ). Empty string if unset.
             */
            const std::string endtime_as_string() const {
                return Osmium::Timestamp::to_iso(m_endtime);
            }

            /**
             * Set the timestamp when this object last changed.
             * @param timestamp Time in seconds since epoch.
             * @return Reference to object to make calls chainable.
             */
            Object& timestamp(time_t timestamp) {
                m_timestamp = timestamp;
                return *this;
            }

            /**
             * Set the endtime after which this object is no longer valid.
             * (This is only used when working with history data.)
             * @param timestamp Time in seconds since epoch.
             * @return Reference to object to make calls chainable.
             */
            Object& endtime(time_t timestamp) {
                m_endtime = timestamp;
                return *this;
            }

            /**
             * Set the timestamp when this object last changed.
             * @param timestamp Timestamp in the format "yyyy-mm-ddThh:mm:ssZ".
             * @return Reference to object to make calls chainable.
             * @exception std::invalid_argument Thrown when the given string
             *   can't be parsed as a timestamp. The object timestamp will remain
             *   unchanged in this case.
             */
            Object& timestamp(const char* timestamp) {
                m_timestamp = Osmium::Timestamp::parse_iso(timestamp);
                return *this;
            }

            /**
             * Get the name of the user who last changed this object.
             * @return Pointer to internal buffer with user name.
             */
            const char* user() const {
                return m_user.c_str();
            }

            /**
             * Set the name of the user who last changed this object.
             * @return Reference to object to make calls chainable.
             * @exception std::length_error Thrown when the username contains more than max_characters_username (255 UTF-8 characters). When the exception is thrown the username is set to "".
             */
            Object& user(const char* user) {
                if (strlen(user) > max_length_username) {
                    throw std::length_error("user name too long");
                }
                m_user = user;
                return *this;
            }

            /**
             * Get the visible flag of this object.
             * (This is only used in OSM files with history.)
             * @return Visible flag.
             */
            bool visible() const {
                return m_visible;
            }

            /**
             * Set the visible flag of this object.
             * (This is only used in OSM files with history.)
             * @return Reference to object to make calls chainable.
             */
            Object& visible(bool visible) {
                m_visible = visible;
                return *this;
            }

            /**
             * Set the visible flag of this object.
             * (This is only used in OSM files with history.)
             * @return Reference to object to make calls chainable.
             */
            Object& visible(const char* visible) {
                if (!strcmp(visible, "false")) {
                    m_visible = false;
                } else {
                    m_visible = true;
                }
                return *this;
            }

            virtual osm_object_type_t type() const = 0;

            /**
             * Set named attribute.
             * @param attr Name of the attribute (must be one of "id", "version", "changeset", "timestamp", "uid", "user", "visible")
             * @param value Value of the attribute
             */
            void set_attribute(const char* attr, const char* value) {
                if (!strcmp(attr, "id")) {
                    id(value);
                } else if (!strcmp(attr, "version")) {
                    version(value);
                } else if (!strcmp(attr, "changeset")) {
                    changeset(value);
                } else if (!strcmp(attr, "timestamp")) {
                    timestamp(value);
                } else if (!strcmp(attr, "uid")) {
                    uid(value);
                } else if (!strcmp(attr, "user")) {
                    user(value);
                } else if (!strcmp(attr, "visible")) {
                    visible(value);
                }
            }

            const TagList& tags() const {
                return m_tags;
            }

            TagList& tags() {
                return m_tags;
            }

            void tags(const TagList& tags) {
                m_tags = tags;
            }

        protected:

            Object() :
                m_id(0),
                m_version(0),
                m_changeset(0),
                m_timestamp(0),
                m_endtime(0),
                m_uid(-1), // to be compatible with Osmosis we use -1 for unknown user id
                m_user(),
                m_visible(true),
                m_tags() {
            }

            Object(const Object& o) :
                m_id(o.m_id),
                m_version(o.m_version),
                m_changeset(o.m_changeset),
                m_timestamp(o.m_timestamp),
                m_endtime(o.m_endtime),
                m_uid(o.m_uid),
                m_user(o.m_user),
                m_visible(o.m_visible),
                m_tags(o.m_tags) {
            }

            virtual ~Object() {
            }

        private:

            osm_object_id_t    m_id;          ///< object id
            osm_version_t      m_version;     ///< object version
            osm_changeset_id_t m_changeset;   ///< id of last changeset that changed this object
            time_t             m_timestamp;   ///< when this object changed last
            time_t             m_endtime;     ///< when this object version was replaced by a new one
            osm_user_id_t      m_uid;         ///< user id of user who last changed this object
            std::string        m_user;        ///< name of user who last changed this object
            bool               m_visible;     ///< object visible (only when working with history data)

            TagList m_tags;

        }; // class Object

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_OBJECT_HPP
