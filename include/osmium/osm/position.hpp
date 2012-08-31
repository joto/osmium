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

#include <cmath>
#include <ostream>
#include <stdint.h>
#include <boost/operators.hpp>
#include <boost/integer_traits.hpp>

namespace Osmium {

    namespace OSM {

        const int coordinate_precision = 10000000;

        namespace {

            inline int32_t double_to_fix(double c) {
                return round(c * coordinate_precision);
            }

            inline double fix_to_double(int32_t c) {
                return static_cast<double>(c) / coordinate_precision;
            }

        }

        /**
         * Positions define a place on earth.
         *
         * Positions are stored in 32 bit integers for the x and y
         * coordinates, respectively. This gives you an accuracy of a few
         * centimeters, good enough for %OSM use. (The main %OSM database
         * uses the same scheme.)
         *
         * An undefined (invalid) Position can be created by calling the
         * constructor without parameters.
         *
         * Coordinates are never checked whether they are inside bounds.
         */
        class Position : boost::totally_ordered<Position> {

        public:

            /// this value is used for a coordinate to mark it as invalid or undefined
            static const int32_t invalid = boost::integer_traits<int32_t>::const_max;

            /**
             * Create undefined Position.
             */
            explicit Position() :
                m_x(invalid),
                m_y(invalid) {
            }

            explicit Position(int32_t x, int32_t y) :
                m_x(x),
                m_y(y) {
            }

            explicit Position(int64_t x, int64_t y) :
                m_x(x),
                m_y(y) {
            }

            explicit Position(double lon, double lat) :
                m_x(double_to_fix(lon)),
                m_y(double_to_fix(lat)) {
            }

            bool defined() const {
                return m_x != invalid && m_y != invalid;
            }

            int32_t x() const {
                return m_x;
            }

            int32_t y() const {
                return m_y;
            }

            Position& x(int32_t x) {
                m_x = x;
                return *this;
            }

            Position& y(int32_t y) {
                m_y = y;
                return *this;
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

        private:

            int32_t m_x;
            int32_t m_y;

        };

        /**
         * Positions are equal if both coordinates are equal.
         */
        inline bool operator==(const Position& p1, const Position& p2) {
            return p1.x() == p2.x() && p1.y() == p2.y();
        }

        /**
         * Compare two positions by comparing first the x and then the
         * y coordinate.
         * If the position is invalid the result is undefined.
         */
        inline bool operator<(const Position& p1, const Position& p2) {
            if (p1.x() == p2.x()) {
                return p1.y() < p2.y();
            } else {
                return p1.x() < p2.x();
            }
        }

        inline std::ostream& operator<<(std::ostream& out, const Position& position) {
            out << '(' << position.lon() << ',' << position.lat() << ')';
            return out;
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_POSITION_HPP
