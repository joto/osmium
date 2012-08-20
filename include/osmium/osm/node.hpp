#ifndef OSMIUM_OSM_NODE_HPP
#define OSMIUM_OSM_NODE_HPP

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
*   @brief Contains the Osmium::OSM::Node class.
*/

#include <boost/operators.hpp>

#include <osmium/osm/position.hpp>
#include <osmium/osm/object.hpp>

namespace Osmium {

    namespace OSM {

        class Node : public Object, boost::less_than_comparable<Node> {

            static const int max_length_coordinate = 12 + 1; ///< maximum length of coordinate string (3 digits + dot + 8 digits + null byte)

            Position m_position;

        public:

            Node() :
                Object(),
                m_position() {
            }

            const Position position() const {
                return m_position;
            }

            Node& position(const Position& position) {
                m_position = position;
                return *this;
            }

            osm_object_type_t type() const {
                return NODE;
            }

            void lon(double x) {
                m_position.lon(x);
            }

            void lat(double y) {
                m_position.lat(y);
            }

            double lon() const {
                return m_position.lon();
            }

            double lat() const {
                return m_position.lat();
            }

        }; // class Node

        /**
         * Nodes can be ordered by id and version.
         * Note that we use the absolute value of the id for a
         * better ordering of objects with negative id.
         */
        inline bool operator<(const Node& lhs, const Node& rhs) {
            if (lhs.id() == rhs.id()) {
                return lhs.version() < rhs.version();
            } else {
                return abs(lhs.id()) < abs(rhs.id());
            }
        }

        /**
         * Ordering for shared_ptrs of Nodes.
         */
        inline bool operator<(const shared_ptr<Node const>& lhs, const shared_ptr<Node const>& rhs) {
            return *lhs < *rhs;
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_NODE_HPP
