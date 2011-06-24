#ifndef OSMIUM_GEOMETRY_POLYGON_HPP
#define OSMIUM_GEOMETRY_POLYGON_HPP

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

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
#endif // OSMIUM_WITH_SHPLIB

#include <osmium/geometry/from_way.hpp>

namespace Osmium {

    namespace Geometry {

        class Polygon : public FromWay {

        public:

            Polygon(const Osmium::OSM::Way& way) : FromWay(way) {
            }

#ifdef  OSMIUM_WITH_SHPLIB
            SHPObject *create_shp_object() {
                return create_shp_line_or_polygon(SHPT_POLYGON);
            }
#endif // OSMIUM_WITH_SHPLIB

        }; // class Polygon

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_POLYGON_HPP
