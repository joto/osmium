#ifndef OSMIUM_UTILS_STRINGTABLE_HPP
#define OSMIUM_UTILS_STRINGTABLE_HPP

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

#include <algorithm>
#include <iostream>
#include <map>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include <osmpbf/osmpbf.h>

namespace Osmium {

    namespace {

        template<typename A, typename B>
        std::pair<B,A> flip_pair(const std::pair<A,B>& p) {
            return std::pair<B,A>(p.second, p.first);
        }

    }

    /**
     * StringTable management for PBF writer
     *
     * All strings are stored as indexes to rows in a StringTable. The StringTable contains
     * one row for each used string, so strings that are used multiple times need to be
     * stored only once. The StringTable is sorted by usage-count, so the most often used
     * string is stored at index 1.
     */
    class StringTable {

        /// type for string IDs (interim and final)
        typedef uint16_t string_id_t;

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
            /// number of occurrences of this string
            uint16_t count;

            /// an intermediate-id
            string_id_t interim_id;
        };

        /**
         * Interim StringTable, storing all strings that should be written to
         * the StringTable once the block is written to disk.
         */
        typedef std::map<std::string, string_info> string2string_info_t;
        string2string_info_t m_strings;

        /**
         * This vector is used to map the interim IDs to real StringTable IDs after
         * writing all strings to the StringTable.
         */
        typedef std::vector<string_id_t> interim_id2id_t;
        interim_id2id_t m_id2id_map;

        int m_size;

    public:

        StringTable() :
            m_strings(),
            m_id2id_map(),
            m_size(0) {
        }

        friend bool operator<(const string_info& lhs, const string_info& rhs) {
            return lhs.count > rhs.count;
        }

        /**
         * record a string in the interim StringTable if it's missing, otherwise just increase its counter,
         * return the interim-id assigned to the string.
         */
        string_id_t record_string(const std::string& string) {
            string_info& info = m_strings[string];
            if (info.interim_id == 0) {
                info.interim_id = ++m_size;
            } else {
                info.count++;
            }
            return info.interim_id;
        }

        /**
         * Sort the interim StringTable and store it to the real protobuf StringTable.
         * while storing to the real table, this function fills the id2id_map with
         * pairs, mapping the interim-ids to final and real StringTable ids.
         *
         * Note that the m_strings table is a std::map and as such is sorted lexicographically.
         * When the transformation into the sortedby multimap is done, it gets sorted by
         * the count. The end result (at least with the glibc standard container/algorithm
         * implementation) is that the string table is sorted first by reverse count (ie descending)
         * and then by reverse lexicographic order.
         */
        void store_stringtable(OSMPBF::StringTable* st) {
            // add empty StringTable entry at index 0
            // StringTable index 0 is reserved as delimiter in the densenodes key/value list
            // this line also ensures that there's always a valid StringTable
            st->add_s("");

            typedef std::multimap<string_info, std::string> cmap;
            cmap sortedbycount;

            m_id2id_map.resize(m_size+1);

            std::transform(m_strings.begin(), m_strings.end(),
                           std::inserter(sortedbycount, sortedbycount.begin()), flip_pair<std::string, string_info>);

            int n=0;
            cmap::const_iterator end=sortedbycount.end();
            for (cmap::const_iterator it = sortedbycount.begin(); it != end; ++it) {
                // add the string of the current item to the pbf StringTable
                st->add_s(it->second);

                // store the mapping from the interim-id to the real id
                m_id2id_map[it->first.interim_id] = ++n;
            }
        }

        /**
         * Map from an interim ID to a real string ID.
         */
        string_id_t map_string_id(const string_id_t interim_id) const {
            return m_id2id_map[interim_id];
        }

        /**
         * Clear the stringtable, preparing for the next block.
         */
        void clear() {
            m_strings.clear();
            m_id2id_map.clear();
            m_size = 0;
        }

    }; // class StringTable

} // namespace Osmium

#endif // OSMIUM_UTILS_STRINGTABLE_HPP
