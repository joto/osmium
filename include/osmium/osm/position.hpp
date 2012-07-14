#ifndef OSMIUM_OSM_POSITION_HPP
#define OSMIUM_OSM_POSITION_HPP

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

#include <stdint.h>
#include <ostream>
#include <limits>
#include <math.h>
#include <boost/operators.hpp>

namespace Osmium {

    namespace OSM {

        /**
        * Positions are stored in 32 bit integers for the x and y
        * coordinates, respectively. This gives you an accuracy of a few
        * centimeters, good enough for OSM use. (The main OSM database uses
        * the same scheme.)
        */
        class Position : boost::totally_ordered<Position> {

        public:

            explicit Position() : m_x(std::numeric_limits<int32_t>::max()), m_y(std::numeric_limits<int32_t>::max()) {
            }

            explicit Position(int32_t x, int32_t y) : m_x(x), m_y(y) {
            }

            explicit Position(double lon, double lat) : m_x(double_to_fix(lon)), m_y(double_to_fix(lat)) {
            }

            bool defined() const {
                return m_x != std::numeric_limits<int32_t>::max() && m_x != std::numeric_limits<int32_t>::min();
            }

            int32_t x() const {
                return m_x;
            }

            int32_t y() const {
                return m_y;
            }

            double lon() const {
                return fix_to_double(m_x);
            }

            double lat() const {
                return fix_to_double(m_y);
            }

            Position& lon(double lon) {
                m_x = double_to_fix(lon);
                return *this;
            }

            Position& lat(double lat) {
                m_y = double_to_fix(lat);
                return *this;
            }

            friend bool operator==(const Position& p1, const Position& p2) {
                return p1.m_x == p2.m_x && p1.m_y == p2.m_y;
            }

            friend bool operator<(const Position& p1, const Position& p2) {
                if (p1.m_x == p2.m_x) {
                    return p1.m_y < p2.m_y;
                } else {
                    return p1.m_x < p2.m_x;
                }
            }

            friend std::ostream& operator<<(std::ostream& out, const Position& position) {
                out << '(' << position.lon() << ',' << position.lat() << ')';
                return out;
            }

            /// conversion to uint32_t
            operator uint32_t() const {
                int32_t x = 180 + m_x / precision;
                int32_t y =  90 - m_y / precision;

                if (x < 0) x = 0;
                if (y < 0) y = 0;
                if (x >= 360) x = 359;
                if (y >= 180) y = 179;

                return 360 * y + x;
            }

        private:

            static const int precision = 10000000;

            int32_t m_x;
            int32_t m_y;

            static int32_t double_to_fix(double c) {
                return round(c * precision);
            }

            static double fix_to_double(int32_t c) {
                return static_cast<double>(c) / precision;
            }

        };

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_POSITION_HPP
