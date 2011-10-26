#ifndef OSMIUM_UTILS_STRINGTABLE_HPP
#define OSMIUM_UTILS_STRINGTABLE_HPP

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

#include <stdint.h>
#include <string>
#include <map>
#include <iostream>

namespace Osmium {

    /**
     * StringTable management for PBF writer
     *
     * All Strings are stored as indexes to rows in a StringTable. The StringTable contains
     * one row for each used string, so strings that are used multiple times need to be
     * stored only once. The StringTable is sorted by usage-count, so the most often used
     * string is stored at index 1.
     */
    class StringTable {

        /**
         * this is the struct used to build the StringTable. It is stored as
         * the value-part in the strings-map.
         *
         * when a new string is added to the map, its count is set to 0 and
         * the interim_id is set to the current size of the map. This interim_id
         * is then stored into the pbf-objects.
         *
         * before the PrimitiveBlock is serialized, the map is sorted by count
         * and stored into the pbf-StringTable. Afterwards the interim-ids are
         * mapped to the "real" id in the StringTable.
         *
         * this way often used strings get lower ids in the StringTable. As the
         * protobuf-serializer stores numbers in variable bit-lengths, lower
         * IDs means less used space in the resulting file.
         */
        struct string_info {
            /**
             * number of occurrences of this string
             */
            uint16_t count;

            /**
             * an intermediate-id
             */
            uint16_t interim_id;
        };

        /**
         * interim StringTable, storing all strings that should be written to
         * the StringTable once the block is written to disk.
         */
        std::map<std::string, string_info> strings;

        /**
         * this map is used to map the interim-ids to real StringTable-IDs after
         * writing all strings to the StringTable.
         */
        std::map<uint16_t, uint16_t> string_ids_map;

        /**
         * this is the comparator used while sorting the interim StringTable.
         */
        static bool stringtable_comparator(const std::pair<std::string, string_info> &a, const std::pair<std::string, string_info> &b) {
            // it first compares based on count
            if (a.second.count > b.second.count) {
                return true;
            } else if (a.second.count < b.second.count) {
                return false;
            } else {
                // if the count is equal, compare based on lexicography order so make
                // the sorting of the later zlib compression faster
                return a.first < b.first;
            }
        }

    public:

        /**
         * record a string in the interim StringTable if it's missing, otherwise just increase its counter,
         * return the interim-id assigned to the string.
         */
        uint16_t record_string(const std::string& string) {
            // try to find the string in the interim StringTable
            if (strings.count(string) > 0) {
                // found, get a pointer to the associated string_info struct
                string_info* info_p = &strings[string];

                // increase the counter by one
                info_p->count++;

                // return the associated interim-id
                //if (Osmium::debug()) fprintf(stderr, "found string %s at interim-id %u\n", string.c_str(), info_p->interim_id);
                return info_p->interim_id;
            } else {
                // not found, initialize a new string_info struct with the count set to 0 and the
                // interim-id set to the current size +1
                string_info info = {0, (uint16_t)(strings.size()+1)};

                // store this string_info struct in the interim StringTable
                strings[string] = info;

                // debug-print and return the associated interim-id
                if (Osmium::debug()) {
                    std::cerr << "record string " << string << " at interim-id " << info.interim_id << std::endl;
                }
                return info.interim_id;
            }
        }

        /**
         * sort the interim StringTable and store it to the real protobuf StringTable.
         * while storing to the real table, this function fills the string_ids_map with
         * pairs, mapping the interim-ids to final and real StringTable ids.
         */
        void store_stringtable(OSMPBF::StringTable* st) {
            // as a map can't be sorted, we need a vector holding our string/string_info pairs
            std::vector<std::pair<std::string, string_info> > strvec;

            // we now copy the contents from the map over to the vector
            std::copy(strings.begin(), strings.end(), back_inserter(strvec));

            // next the vector is sorted using our comparator
            std::sort(strvec.begin(), strvec.end(), stringtable_comparator);

            // iterate over the items of our vector
            for (int i=0, l=strvec.size(); i<l; i++) {
                if (Osmium::debug()) {
                    std::cerr << "store stringtable: " << strvec[i].first << " (cnt=" << strvec[i].second.count+1 << ") with interim-id " << strvec[i].second.interim_id << " at stringtable-id " << i+1 << std::endl;
                }

                // add the string of the current item to the pbf StringTable
                st->add_s(strvec[i].first);

                // store the mapping from the interim-id to the real id
                string_ids_map[strvec[i].second.interim_id] = i+1;
            }
        }

        /**
         * map from an interim-id to a real string-id, throwing an exception if an
         * unknown mapping is requested.
         */
        uint16_t map_string_id(const uint16_t interim_id) {
            // declare an iterator over the ids-map
            std::map<uint16_t, uint16_t>::const_iterator it;

            // use the find method of the map to search for the mapping pair
            it = string_ids_map.find(interim_id);

            // if there was a hit
            if (it != string_ids_map.end()) {
                // return the real-id stored in the second part of the pair
                //if (Osmium::debug()) fprintf(stderr, "mapping interim-id %u to stringtable-id %u\n", interim_id, it->second);
                return it->second;
            }

            // throw an exception
            throw std::runtime_error("Request for string not in stringable\n");
        }

        /**
         * clear the stringtable, preparing for the next block
         */
        void clear() {
            strings.clear();
            string_ids_map.clear();
        }

    }; // class StringTable

} // namespace Osmium

#endif // OSMIUM_UTILS_STRINGTABLE_HPP
