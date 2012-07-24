#ifndef OSMIUM_OSM_AREA_HPP
#define OSMIUM_OSM_AREA_HPP

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

#include <boost/operators.hpp>

#include <osmium/osm/object.hpp>
#include <osmium/osm/way.hpp>

namespace geos {
    namespace geom {
        class MultiPolygon;
    }
}

namespace Osmium {

    namespace OSM {

        class Area : public Object, boost::less_than_comparable<Area> {

            WayNodeList m_node_list;
            geos::geom::MultiPolygon* m_geos_geometry;

        public:

            /// Construct an Area object.
            Area() :
                Object(),
                m_node_list(),
                m_geos_geometry(0) {
            }

            /// Construct an Area object from a Way object.
            Area(const Way& way) :
                Object(way),
                m_node_list(way.nodes()),
                m_geos_geometry(0) {
            }

            /// Copy an Area object.
            Area(const Area& area) :
                Object(area),
                m_node_list(area.m_node_list),
                m_geos_geometry(area.m_geos_geometry) {
            }

            osm_object_type_t get_type() const {
                return AREA;
            }

            const WayNodeList& nodes() const {
                return m_node_list;
            }

            WayNodeList& nodes() {
                return m_node_list;
            }

            geos::geom::MultiPolygon* geos_geometry() const {
                return m_geos_geometry;
            }

            Area& geos_geometry(geos::geom::MultiPolygon* geometry) {
                m_geos_geometry = geometry;
                return *this;
            }

            /**
             * Areas can be ordered by id and version.
             * Note that we use the absolute value of the id for a
             * better ordering of objects with negative id.
             */
            friend bool operator<(const Area& lhs, const Area& rhs) {
                if (lhs.id() == rhs.id()) {
                    return lhs.version() < rhs.version();
                } else {
                    return abs(lhs.id()) < abs(rhs.id());
                }
            }

            /**
             * Ordering for shared_ptrs of Areas.
             */
            friend bool operator<(const shared_ptr<Area const>& lhs, const shared_ptr<Area const>& rhs) {
                return *lhs < *rhs;
            }

        }; // class Area

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_AREA_HPP
