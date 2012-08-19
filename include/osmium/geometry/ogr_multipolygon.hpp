#ifndef OSMIUM_GEOMETRY_OGR_MULTIPOLYGON_HPP
#define OSMIUM_GEOMETRY_OGR_MULTIPOLYGON_HPP

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

#include <ogr_geometry.h>

#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>

#include <osmium/geometry/multipolygon.hpp>

namespace Osmium {

    namespace Geometry {

        namespace {

            inline void add_ring(OGRPolygon* ogrpolygon, const geos::geom::LineString* geosring) {
                OGRLinearRing* ogrring = new OGRLinearRing;

                const geos::geom::CoordinateSequence* cs = geosring->getCoordinatesRO();
                ogrring->setNumPoints(cs->getSize());

                for (size_t i = 0; i < cs->getSize(); ++i) {
                    ogrring->setPoint(i, cs->getX(i), cs->getY(i));
                }

                ogrpolygon->addRingDirectly(ogrring);
            }

            inline OGRPolygon* make_polygon(const geos::geom::Polygon* geospolygon) {
                OGRPolygon* ogrpolygon = new OGRPolygon;

                add_ring(ogrpolygon, geospolygon->getExteriorRing());
                for (size_t i=0; i < geospolygon->getNumInteriorRing(); ++i) {
                    add_ring(ogrpolygon, geospolygon->getInteriorRingN(i));
                }

                return ogrpolygon;
            }

        }

        /**
         * Create OGR geometry of a MultiPolygon.
         *
         * Caller takes ownership.
         */
        inline OGRMultiPolygon* create_ogr_geometry(const Osmium::Geometry::MultiPolygon& multipolygon) {
            OGRMultiPolygon* ogrmp = new OGRMultiPolygon;

            const geos::geom::GeometryCollection* geosgeom = dynamic_cast<const geos::geom::GeometryCollection*>(multipolygon.borrow_geos_geometry());
            for (geos::geom::GeometryCollection::const_iterator it = geosgeom->begin(); it != geosgeom->end(); ++it) {

                OGRPolygon* ogrpolygon = make_polygon(dynamic_cast<const geos::geom::Polygon*>(*it));

                OGRErr result = ogrmp->addGeometryDirectly(ogrpolygon);
                if (result != OGRERR_NONE) {
                    // delete ogrpolygon;    XXX are we supposed to delete this ourselves?
                    delete ogrmp;
                    throw Osmium::Geometry::IllegalGeometry();
                }
            }

            return ogrmp;
        }

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_OGR_MULTIPOLYGON_HPP
