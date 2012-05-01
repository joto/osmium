#ifndef OSMIUM_OSM_BOUNDS_HPP
#define OSMIUM_OSM_BOUNDS_HPP

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

#include <limits>

#include <osmium/osm/position.hpp>

namespace Osmium {

    namespace OSM {

        class Bounds {

        public:

            Bounds()
                : m_min_x(std::numeric_limits<int32_t>::max()),
                  m_max_x(std::numeric_limits<int32_t>::min()),
                  m_min_y(std::numeric_limits<int32_t>::max()),
                  m_max_y(std::numeric_limits<int32_t>::min()) {
            }

            Bounds& extend(const Position& position) {
                if (position.x() < m_min_x) m_min_x = position.x();
                if (position.x() > m_max_x) m_max_x = position.x();
                if (position.y() < m_min_y) m_min_y = position.y();
                if (position.y() > m_max_y) m_max_y = position.y();
                return *this;
            }

            bool defined() const {
                return m_min_x != std::numeric_limits<int32_t>::max();
            }

            /**
             * Bottom-left position.
             */
            Position bl() const {
                return Position(m_min_x, m_min_y);
            }

            /**
             * Top-right position.
             */
            Position tr() const {
                return Position(m_max_x, m_max_y);
            }

            friend std::ostream& operator<<(std::ostream& out, const Bounds& bounds) {
                out << '(' << bounds.bl().lon() << ',' << bounds.bl().lat() << ','
                    << bounds.tr().lon() << ',' << bounds.tr().lat() << ')';
                return out;
            }

        private:

            int32_t m_min_x;
            int32_t m_max_x;
            int32_t m_min_y;
            int32_t m_max_y;

        }; // class Bounds

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_BOUNDS_HPP
