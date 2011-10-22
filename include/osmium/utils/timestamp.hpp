#ifndef OSMIUM_UTILS_TIMESTAMP_HPP
#define OSMIUM_UTILS_TIMESTAMP_HPP

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

#include <stdexcept>
#include <time.h>

namespace Osmium {

    namespace Utils {

        /**
         * Contains some helper functions to convert timestamps from time_t to
         * the ISO format used by OSM and back.
         */
        class Timestamp {

            static const int timestamp_length = 20 + 1; // length of ISO timestamp string yyyy-mm-ddThh:mm:ssZ\0

            /**
             * The timestamp format for OSM timestamps in strftime(3) format.
             * This is the ISO-Format yyyy-mm-ddThh:mm:ssZ
             */
            static const char* timestamp_format() {
                static const char f[] = "%Y-%m-%dT%H:%M:%SZ";
                return f;
            }

            /// Constructor is private, this class is not supposed to be instantiated
            Timestamp() {
            }

        public:

            static std::string to_iso(time_t timestamp) {
                if (timestamp == 0) {
                    return std::string("");
                }
                struct tm* tm = gmtime(&timestamp);
                std::string s(timestamp_length, '\0');
                /* This const_cast is ok, because we know we have enough space
                   in the string for the format we are using (well at least until
                   the year will have 5 digits). And by setting the size
                   afterwards from the result of strftime we make sure thats set
                   right, too. */
                s.resize(strftime(const_cast<char *>(s.c_str()), timestamp_length, timestamp_format(), tm));
                return s;
            }

            static time_t parse_iso(const char* timestamp) {
                struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                if (strptime(timestamp, Osmium::Utils::Timestamp::timestamp_format(), &tm) == NULL) {
                    throw std::invalid_argument("can't parse timestamp");
                }
                return timegm(&tm);
            }

        }; // class Timestamp

    } // namespace Utils

} // namespace Osmium

#endif // OSMIUM_UTILS_TIMESTAMP_HPP
