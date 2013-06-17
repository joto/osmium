#ifndef OSMIUM_UTILS_TIMESTAMP_HPP
#define OSMIUM_UTILS_TIMESTAMP_HPP

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

#include <ctime>
#include <stdexcept>
#include <string>

namespace Osmium {

    /**
     * Contains some helper functions to convert timestamps from time_t to
     * the ISO format used by OSM and back.
     */
    namespace Timestamp {

        namespace {

            static const int timestamp_length = 20 + 1; // length of ISO timestamp string yyyy-mm-ddThh:mm:ssZ\0

            /**
            * The timestamp format for OSM timestamps in strftime(3) format.
            * This is the ISO-Format yyyy-mm-ddThh:mm:ssZ
            */
            inline const char* timestamp_format() {
                static const char f[] = "%Y-%m-%dT%H:%M:%SZ";
                return f;
            }
        }

        /**
         * Return UTC Unix time as string in ISO date/time format.
         */
        inline std::string to_iso(time_t timestamp) {
            if (timestamp == 0) {
                return std::string("");
            }
            struct tm* tm = std::gmtime(&timestamp);
            std::string s(timestamp_length, '\0');
            /* This const_cast is ok, because we know we have enough space
               in the string for the format we are using (well at least until
               the year will have 5 digits). And by setting the size
               afterwards from the result of strftime we make sure thats set
               right, too. */
            s.resize(strftime(const_cast<char*>(s.c_str()), timestamp_length, timestamp_format(), tm));
            return s;
        }

        /**
         * Parse ISO date/time string and return UTC unix time.
         * Throws std::invalid_argument, if the timestamp can not be parsed.
         */
        inline time_t parse_iso(const char* timestamp) {
#ifndef WIN32
            struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
            if (strptime(timestamp, timestamp_format(), &tm) == NULL) {
                throw std::invalid_argument("can't parse timestamp");
            }
            return timegm(&tm);
#else
            struct tm tm;
            int n = sscanf(timestamp, "%4d-%2d-%2dT%2d:%2d:%2dZ", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            if (n != 6) {
                throw std::invalid_argument("can't parse timestamp");
            }
            tm.tm_year -= 1900;
            tm.tm_mon--;
            tm.tm_wday = 0;
            tm.tm_yday = 0;
            tm.tm_isdst = 0;
            return _mkgmtime(&tm);
#endif
        }

    } // namespace Timestamp

} // namespace Osmium

#endif // OSMIUM_UTILS_TIMESTAMP_HPP
