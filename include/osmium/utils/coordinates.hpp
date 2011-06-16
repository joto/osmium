#ifndef OSMIUM_UTILS_COORDINATES_HPP
#define OSMIUM_UTILS_COORDINATES_HPP

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

namespace Osmium {

    /**
     * A helper class representing a location.
     * Locations are stored in 32 bit integers for the
     * x and y coordinates, respectively. This gives you an
     * accuracy of a few centimeters, good enough for OSM use.
     * (The main OSM database uses the same scheme.)
     */
    class Coordinates {

    public:

        Coordinates() : m_x(0), m_y(0) {
        }

        explicit Coordinates(const Osmium::OSM::Node &node) {
            m_x = double_to_fix(node.get_lon());
            m_y = double_to_fix(node.get_lat());
        }

        double x() const {
            return fix_to_double(m_x);
        }

        double y() const {
            return fix_to_double(m_y);
        }

    private:

        int32_t m_x;
        int32_t m_y;

        static int32_t double_to_fix(double c) {
            return c * 10000000;
        }

        static double fix_to_double(int32_t c) {
            return ((double)c) / 10000000;
        }

    };

} // namespace Osmium

#endif // OSMIUM_UTILS_COORDINATES_HPP
