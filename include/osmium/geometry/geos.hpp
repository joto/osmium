#ifndef OSMIUM_GEOMETRY_GEOS_HPP
#define OSMIUM_GEOMETRY_GEOS_HPP

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

#define OSMIUM_COMPILE_WITH_CFLAGS_GEOS `geos-config --cflags`
#define OSMIUM_LINK_WITH_LIBS_GEOS `geos-config --libs`

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>

#include <osmium/osm/position.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/geometry/linestring.hpp>
#include <osmium/geometry/polygon.hpp>

namespace Osmium {

    namespace Geometry {

        /**
         * Return pointer to a static GEOS GeometryFactory object created the
         * first time this function is run. This is used by all functions in
         * Osmium that need to create GEOS geometries.
         */
        inline geos::geom::GeometryFactory* geos_geometry_factory() {
            static geos::geom::PrecisionModel pm;
            static geos::geom::GeometryFactory factory(&pm, -1);
            return &factory;
        }

        /**
         * Creates GEOS coordinate from a Position
         */
        inline geos::geom::Coordinate create_geos_coordinate(const Osmium::OSM::Position position) {
            return geos::geom::Coordinate(position.lon(), position.lat());
        }

        /**
         * Creates GEOS geometry of a Point.
         *
         * Caller takes ownership.
         */
        inline geos::geom::Point* create_geos_geometry(const Osmium::Geometry::Point& point) {
            return Osmium::Geometry::geos_geometry_factory()->createPoint(Osmium::Geometry::create_geos_coordinate(point.position()));
        }

        /**
         * Create GEOS geometry of a LineString.
         *
         * Caller takes ownership.
         */
        inline geos::geom::LineString* create_geos_geometry(const Osmium::Geometry::LineString& linestring) {
            std::vector<geos::geom::Coordinate>* c = new std::vector<geos::geom::Coordinate>;
            if (linestring.reverse()) {
                for (Osmium::OSM::WayNodeList::const_reverse_iterator it = linestring.nodes().rbegin(); it != linestring.nodes().rend(); ++it) {
                    c->push_back(Osmium::Geometry::create_geos_coordinate(it->position()));
                }
            } else {
                for (Osmium::OSM::WayNodeList::const_iterator it = linestring.nodes().begin(); it != linestring.nodes().end(); ++it) {
                    c->push_back(Osmium::Geometry::create_geos_coordinate(it->position()));
                }
            }
            geos::geom::CoordinateSequence* cs = Osmium::Geometry::geos_geometry_factory()->getCoordinateSequenceFactory()->create(c);
            return Osmium::Geometry::geos_geometry_factory()->createLineString(cs);
        }

        /**
         * Creates GEOS geometry of a Polygon.
         *
         * Caller takes ownership.
         */
        inline geos::geom::Polygon* create_geos_geometry(const Osmium::Geometry::Polygon& polygon) {
            std::vector<geos::geom::Coordinate>* c = new std::vector<geos::geom::Coordinate>;
            if (polygon.reverse()) {
                for (Osmium::OSM::WayNodeList::const_reverse_iterator it = polygon.nodes().rbegin(); it != polygon.nodes().rend(); ++it) {
                    c->push_back(Osmium::Geometry::create_geos_coordinate(it->position()));
                }
            } else {
                for (Osmium::OSM::WayNodeList::const_iterator it = polygon.nodes().begin(); it != polygon.nodes().end(); ++it) {
                    c->push_back(Osmium::Geometry::create_geos_coordinate(it->position()));
                }
            }
            geos::geom::CoordinateSequence* cs = Osmium::Geometry::geos_geometry_factory()->getCoordinateSequenceFactory()->create(c);
            geos::geom::LinearRing* lr = Osmium::Geometry::geos_geometry_factory()->createLinearRing(cs);
            return Osmium::Geometry::geos_geometry_factory()->createPolygon(lr, NULL);
        }

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_GEOS_HPP
