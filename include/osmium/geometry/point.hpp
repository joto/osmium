#ifndef OSMIUM_GEOMETRY_POINT_HPP
#define OSMIUM_GEOMETRY_POINT_HPP

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

#include <vector>
#include <sstream>
#include <iomanip>

#ifdef OSMIUM_WITH_GEOS
# include <geos/geom/Point.h>
#endif // OSMIUM_WITH_GEOS

#include <osmium/geometry.hpp>
#include <osmium/osm/node.hpp>

namespace Osmium {

    namespace Geometry {

        /**
         * Point geometry.
         */
        class Point : public Geometry {

        public:

            /**
             * Create point geometry from a position.
             */
            Point(const Osmium::OSM::Position& position, osm_object_id_t id=0) : Geometry(id), m_position(position) {
            }

            /**
             * Create point geometry from position of a node.
             */
            Point(const Osmium::OSM::Node& node)
                : Geometry(node.id()),
                  m_position(Osmium::OSM::Position(node.get_lon(), node.get_lat())) {
            }

            const Osmium::OSM::Position& position() const {
                return m_position;
            }

            double lon() const {
                return m_position.lon();
            }

            double lat() const {
                return m_position.lat();
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                return out << "POINT(" << std::setprecision(10) << lon() << " " << lat() << ")";
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                write_binary_wkb_header(out, with_srid, wkbPoint);
                write_binary<double>(out, lon());
                write_binary<double>(out, lat());
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                write_hex_wkb_header(out, with_srid, wkbPoint);
                write_hex<double>(out, lon());
                write_hex<double>(out, lat());
                return out;
            }

#ifdef OSMIUM_WITH_GEOS
            /**
             * Creates GEOS geometry of this Point.
             *
             * Caller takes ownership.
             */
            geos::geom::Point* create_geos_geometry() const {
                return Osmium::Geometry::geos_geometry_factory()->createPoint(m_position);
            }
#endif // OSMIUM_WITH_GEOS

#ifdef OSMIUM_WITH_SHPLIB
            /**
             * Create Shapelib geometry of this Point.
             *
             * Caller takes ownership. You have to call
             * SHPDestroyObject() with this geometry when you are done.
             */
            SHPObject* create_shp_object() const {
                double x = lon();
                double y = lat();
                return SHPCreateSimpleObject(SHPT_POINT, 1, &x, &y, NULL);
            }
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_OGR
            /**
             * Create OGR geometry of this Point.
             *
             * Caller takes ownership.
             */
            OGRPoint* create_ogr_geometry() const {
                OGRPoint* p = new OGRPoint(lon(), lat());
                return p;
            }
#endif // OSMIUM_WITH_OGR

        private:

            const Osmium::OSM::Position m_position;

        }; // class Point

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_POINT_HPP
