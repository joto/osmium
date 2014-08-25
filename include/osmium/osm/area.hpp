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

#include <geos/geom/MultiPolygon.h>

#include <osmium/osm/object.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>

namespace Osmium {

    namespace Geometry {
        class MultiPolygon;
    }

    namespace {

        template <typename T>
        int sgn(T val) {
            return (T(0) < val) - (val < T(0));
        }

    }

    namespace OSM {

        /**
         * Area objects are pseudo OSM objects created from Ways or
         * Relations of type=multipolygon (or type=boundary).
         *
         * Area IDs are unique regardless of whether the Area was created
         * from a Way or from a Relation.
         */
        class Area : public Object, boost::less_than_comparable<Area> {

            friend class Osmium::Geometry::MultiPolygon;

            WayNodeList m_node_list;
            mutable geos::geom::MultiPolygon* m_geos_geometry;

            const geos::geom::MultiPolygon* geos_geometry() const {
                return m_geos_geometry;
            }

        public:

            /// Construct an Area object from a Relation object.
            Area(const Relation& relation) :
                Object(relation),
                m_node_list(),
                m_geos_geometry() {
                id((id() * 2) + sgn(id()));
            }

            /// Construct an Area object from a Way object.
            Area(const Way& way) :
                Object(way),
                m_node_list(way.nodes()),
                m_geos_geometry() {
                id(id() * 2);
            }

            Area(const Area& area) :
                Object(area),
                m_node_list(area.m_node_list),
                m_geos_geometry(dynamic_cast<geos::geom::MultiPolygon*>(area.m_geos_geometry->clone())) {
            }

            Area& operator=(const Area& area) {
                id(area.id());
                version(area.version());
                changeset(area.changeset());
                timestamp(area.timestamp());
                endtime(area.endtime());
                uid(area.uid());
                user(area.user());
                visible(area.visible());
                tags(area.tags());
                m_node_list = area.m_node_list;
                m_geos_geometry = dynamic_cast<geos::geom::MultiPolygon*>(area.m_geos_geometry->clone());
                return *this;
            }

            ~Area() {
                delete m_geos_geometry;
            }

            osm_object_type_t type() const {
                return AREA;
            }

            /// Was this Area created from a Way or Relation?
            bool from_way() const {
                return (id() % 2) == 0;
            }

            /// ID of the Way or Relation objects this Area was created from.
            osm_object_id_t orig_id() const {
                return id() / 2;
            }

            const Area& geos_geometry(geos::geom::MultiPolygon* multipolygon) const {
                m_geos_geometry = multipolygon;
                return *this;
            }

        }; // class Area

        /**
         * Areas can be ordered by id and version.
         * Note that we use the absolute value of the id for a
         * better ordering of objects with negative id.
         */
        inline bool operator<(const Area& lhs, const Area& rhs) {
            if (lhs.id() == rhs.id()) {
                return lhs.version() < rhs.version();
            } else {
                return std::abs(lhs.id()) < std::abs(rhs.id());
            }
        }

        /**
         * Ordering for shared_ptrs of Areas.
         */
        inline bool operator<(const shared_ptr<Area const>& lhs, const shared_ptr<Area const>& rhs) {
            return *lhs < *rhs;
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_AREA_HPP
