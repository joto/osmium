#ifndef OSMIUM_OSM_UNDIRECTED_SEGMENT_HPP
#define OSMIUM_OSM_UNDIRECTED_SEGMENT_HPP

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

#include <osmium/osm/segment.hpp>

namespace Osmium {

    namespace OSM {

        /**
         * Undirected connection between two Positions. The first Position is
         * always equal or "smaller" than the second Position, ie to the left
         * and down.
         */
        class UndirectedSegment : boost::less_than_comparable<UndirectedSegment>, public Segment {

        public:

            UndirectedSegment(const Osmium::OSM::Position& p1, const Osmium::OSM::Position& p2) :
                Segment(p1, p2) {
                if (p2 < p1) {
                    swap_positions();
                }
            }

        }; // class UndirectedSegment

        /**
        * UndirectedSegments are "smaller" if they are to the left and down of another
        * segment. The first() position is checked first() and only if they have the
        * same first() position the second() position is taken into account.
        */
        inline bool operator<(const UndirectedSegment& lhs, const UndirectedSegment& rhs) {
            if (lhs.first() == rhs.first()) {
                return lhs.second() < rhs.second();
            } else {
                return lhs.first() < rhs.first();
            }
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_UNDIRECTED_SEGMENT_HPP
