#ifndef OSMIUM_OSM_SEGMENT_HPP
#define OSMIUM_OSM_SEGMENT_HPP

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

#include <osmium/osm/position.hpp>

namespace Osmium {

    namespace OSM {

        /**
         * A Segment is the directed connection between two Positions.
         */
        class Segment : boost::equality_comparable<Segment> {

        public:

            Segment(const Osmium::OSM::Position& p1, const Osmium::OSM::Position& p2) :
                m_first(p1),
                m_second(p2) {
            }

            /// Return first Position of Segment.
            const Osmium::OSM::Position first() const {
                return m_first;
            }

            /// Return second Position of Segment.
            const Osmium::OSM::Position second() const {
                return m_second;
            }

        protected:

            void swap_positions() {
                std::swap(m_first, m_second);
            }

        private:

            Osmium::OSM::Position m_first;
            Osmium::OSM::Position m_second;

        }; // class Segment

        /// Segments are equal if both their positions are equal
        inline bool operator==(const Segment& lhs, const Segment& rhs) {
            return lhs.first() == rhs.first() && lhs.second() == rhs.second();
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_SEGMENT_HPP
