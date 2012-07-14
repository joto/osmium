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

/*
=======================================================
  If you include this header file, you need to
  compile with: gdal-config --cflags
  and
  link with: gdal-config --libs
=======================================================
*/

#include <boost/foreach.hpp>

#include <ogr_geometry.h>

#include <osmium/geometry/point.hpp>
#include <osmium/geometry/linestring.hpp>
#include <osmium/geometry/polygon.hpp>
#include <osmium/geometry/multipolygon.hpp>

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

#ifdef OSMIUM_WITH_GEOS
        inline void add_ring(OGRPolygon* ogrpolygon, const geos::geom::LineString* geosring) {
            OGRLinearRing* ogrring = new OGRLinearRing;

            const geos::geom::CoordinateSequence* cs = geosring->getCoordinatesRO();
            ogrring->setNumPoints(cs->getSize());

            for (size_t i = 0; i < cs->getSize(); ++i) {
                ogrring->setPoint(i, cs->getX(i), cs->getY(i), 0);
            }

            ogrpolygon->addRingDirectly(ogrring);
        };

        inline OGRPolygon* make_polygon(const geos::geom::Polygon* geospolygon) {
            OGRPolygon* ogrpolygon = new OGRPolygon;

            add_ring(ogrpolygon, geospolygon->getExteriorRing());
            for (size_t i=0; i < geospolygon->getNumInteriorRing(); ++i) {
                add_ring(ogrpolygon, geospolygon->getInteriorRingN(i));
            }

            return ogrpolygon;
        }

        /**
         * Create OGR geometry of a MultiPolygon.
         *
         * Caller takes ownership.
         */
        inline OGRMultiPolygon* create_ogr_geometry(const Osmium::Geometry::MultiPolygon& multipolygon) {
            OGRMultiPolygon* ogrmp = new OGRMultiPolygon;

            if (multipolygon.area()->get_geometry()->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
                OGRPolygon* ogrpolygon = make_polygon(dynamic_cast<const geos::geom::Polygon*>(multipolygon.area()->get_geometry()));

                OGRErr result = ogrmp->addGeometryDirectly(ogrpolygon);
                if (result != OGRERR_NONE) {
                    throw Osmium::Exception::IllegalGeometry();
                }
                return ogrmp;
            }

            if (multipolygon.area()->get_geometry()->getGeometryTypeId() != geos::geom::GEOS_MULTIPOLYGON) {
                throw Osmium::Exception::IllegalGeometry();
            }

            const geos::geom::GeometryCollection* geosgeom = dynamic_cast<const geos::geom::GeometryCollection*>(multipolygon.area()->get_geometry());
            for (geos::geom::GeometryCollection::const_iterator it = geosgeom->begin(); it != geosgeom->end(); ++it) {

                OGRPolygon* ogrpolygon = make_polygon(dynamic_cast<const geos::geom::Polygon*>(*it));

                OGRErr result = ogrmp->addGeometryDirectly(ogrpolygon);
                if (result != OGRERR_NONE) {
                    throw Osmium::Exception::IllegalGeometry();
                }
            }

            return ogrmp;
        }
#endif // OSMIUM_WITH_GEOS

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_OGR_HPP
