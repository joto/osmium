#ifndef OSMIUM_OSM_WAY_HPP
#define OSMIUM_OSM_WAY_HPP

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
*   @brief Contains the Osmium::OSM::Way class.
*/

#include <boost/operators.hpp>

#include <osmium/osm/object.hpp>
#include <osmium/osm/way_node_list.hpp>
#include <osmium/geometry.hpp>

namespace Osmium {

    namespace OSM {

        class Way : public Object, boost::less_than_comparable<Way> {

            WayNodeList m_node_list;

        public:

            Way() :
                Object(),
                m_node_list() {
            }

            Way(int size_of_node_list) :
                Object(),
                m_node_list(size_of_node_list) {
            }

            Way(const Way& way) :
                Object(way),
                m_node_list(way.m_node_list) {
            }

            const WayNodeList& nodes() const {
                return m_node_list;
            }

            WayNodeList& nodes() {
                return m_node_list;
            }

            osm_object_type_t type() const {
                return WAY;
            }

            osm_object_id_t get_node_id(osm_sequence_id_t n) const {
                return m_node_list[n].ref();
            }

            /**
            * Add a node with the given id to the way.
            *
            * Will throw a range error if the way already has max_nodes_in_way nodes.
            */
            void add_node(osm_object_id_t ref) {
                m_node_list.add(ref);
            }

            /**
             * Returns the id of the first node.
             */
            osm_object_id_t get_first_node_id() const {
                return m_node_list.front().ref();
            }

            /**
             * Returns the id of the last node.
             */
            osm_object_id_t get_last_node_id() const {
                return m_node_list.back().ref();
            }

            /**
            * Check whether this way is closed. A way is closed if the first and last node have the same id.
            */
            bool is_closed() const {
                return m_node_list.is_closed();
            }

        }; // class Way

        /**
         * Ways can be ordered by id and version.
         * Note that we use the absolute value of the id for a
         * better ordering of objects with negative ids.
         */
        inline bool operator<(const Way& lhs, const Way& rhs) {
            if (lhs.id() == rhs.id()) {
                return lhs.version() < rhs.version();
            } else {
                return abs(lhs.id()) < abs(rhs.id());
            }
        }

        /**
         * Ordering for shared_ptrs of Ways.
         */
        inline bool operator<(const shared_ptr<Way const>& lhs, const shared_ptr<Way const>& rhs) {
            return *lhs < *rhs;
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_WAY_HPP
