#ifndef OSMIUM_GEOMETRY_OGR_HPP
#define OSMIUM_GEOMETRY_OGR_HPP

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

#define OSMIUM_COMPILE_WITH_CFLAGS_OGR `gdal-config --cflags`
#define OSMIUM_LINK_WITH_LIBS_OGR `gdal-config --libs`

#include <boost/foreach.hpp>

#include <ogr_geometry.h>

#include <osmium/geometry/point.hpp>
#include <osmium/geometry/linestring.hpp>
#include <osmium/geometry/polygon.hpp>

namespace Osmium {

    namespace Geometry {

        /**
         * Create OGR geometry of a Point.
         *
         * Caller takes ownership.
         */
        inline OGRPoint* create_ogr_geometry(const Osmium::Geometry::Point& point) {
            return new OGRPoint(point.lon(), point.lat());
        }

        /**
         * Create OGR geometry of a LineString;
         *
         * Caller takes ownership.
         */
        inline OGRLineString* create_ogr_geometry(const Osmium::Geometry::LineString& linestring) {
            OGRLineString* ls = new OGRLineString();
            if (linestring.reverse()) {
                BOOST_REVERSE_FOREACH(const Osmium::OSM::WayNode& wn, linestring.nodes()) {
                    ls->addPoint(wn.lon(), wn.lat());
                }
            } else {
                BOOST_FOREACH(const Osmium::OSM::WayNode& wn, linestring.nodes()) {
                    ls->addPoint(wn.lon(), wn.lat());
                }
            }
            return ls;
        }

        /**
         * Create OGR geometry of a Polygon.
         *
         * Caller takes ownership.
         */
        inline OGRPolygon* create_ogr_geometry(const Osmium::Geometry::Polygon& polygon) {
            OGRPolygon* p = new OGRPolygon();
            OGRLinearRing* r = new OGRLinearRing();
            if (polygon.reverse()) {
                BOOST_REVERSE_FOREACH(const Osmium::OSM::WayNode& wn, polygon.nodes()) {
                    r->addPoint(wn.lon(), wn.lat());
                }
            } else {
                BOOST_FOREACH(const Osmium::OSM::WayNode& wn, polygon.nodes()) {
                    r->addPoint(wn.lon(), wn.lat());
                }
            }
            p->addRingDirectly(r);
            return p;
        }

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_OGR_HPP
