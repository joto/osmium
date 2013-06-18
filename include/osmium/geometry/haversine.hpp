#ifndef OSMIUM_GEOMETRY_HAVERSINE_HPP
#define OSMIUM_GEOMETRY_HAVERSINE_HPP

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
#include <osmium/osm/way_node_list.hpp>

namespace Osmium {

    namespace Geometry {

        /**
         * @brief Functions to calculate arc distance on Earth using the haversine formula.
         *
         * See http://en.wikipedia.org/wiki/Haversine_formula
         *
         * Implementation derived from
         * http://blog.julien.cayzac.name/2008/10/arc-and-distance-between-two-points-on.html
         */
        namespace Haversine {

            /// @brief The usual PI/180 constant
            static const double DEG_TO_RAD = 0.017453292519943295769236907684886;

            /// @brief Earth's quadratic mean radius for WGS84
            static const double EARTH_RADIUS_IN_METERS = 6372797.560856;

            inline double distance(const double x1, const double y1, const double x2, const double y2) {
                const double lon_arc = (x1 - x2) * DEG_TO_RAD;
                const double lat_arc = (y1 - y2) * DEG_TO_RAD;
                double lonh = sin(lon_arc * 0.5);
                lonh *= lonh;
                double lath = sin(lat_arc * 0.5);
                lath *= lath;
                const double tmp = cos(y1 * DEG_TO_RAD) * cos(y2 * DEG_TO_RAD);
                return 2.0 * EARTH_RADIUS_IN_METERS * asin(sqrt(lath + tmp*lonh));
            }

            inline double distance(const Osmium::OSM::Position& from, const Osmium::OSM::Position& to) {
                return distance(from.lon(), from.lat(), to.lon(), to.lat());
            }

            inline double distance(const Osmium::OSM::WayNodeList& wnl) {
                double sum_length=0;

                if (wnl.size() < 2) {
                    return 0.0;
                }

                for (Osmium::OSM::WayNodeList::const_iterator it = wnl.begin(); it != wnl.end()-1; ++it) {
                    sum_length += Osmium::Geometry::Haversine::distance(it->position(), (it+1)->position());
                }

                return sum_length;
            }

        } // namespace Haversine

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_HAVERSINE_HPP
