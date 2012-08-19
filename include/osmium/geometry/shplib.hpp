#ifndef OSMIUM_GEOMETRY_SHPLIB_HPP
#define OSMIUM_GEOMETRY_SHPLIB_HPP

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

#define OSMIUM_COMPILE_WITH_CFLAGS_SHP `geos-config --cflags`
#define OSMIUM_LINK_WITH_LIBS_SHP -lshp `geos-config --libs`

#include <vector>
#include <shapefil.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>

#include <osmium/geometry/point.hpp>
#include <osmium/geometry/linestring.hpp>
#include <osmium/geometry/polygon.hpp>
#include <osmium/geometry/multipolygon.hpp>

namespace Osmium {

    namespace Geometry {

        /**
         * Create Shapelib geometry of a Point.
         *
         * Caller takes ownership. You have to call
         * SHPDestroyObject() with this geometry when you are done.
         */
        inline SHPObject* create_shp_object(const Osmium::Geometry::Point& point) {
            double x = point.lon();
            double y = point.lat();
            return SHPCreateSimpleObject(SHPT_POINT, 1, &x, &y, NULL);
        }

        namespace {

            /**
            * Create a SHPObject for a Geometry created from a way and return it.
            *
            * Caller takes ownership. You have to call
            * SHPDestroyObject() with this geometry when you are done.
            */
            inline SHPObject* create_line_or_polygon(const Osmium::Geometry::FromWay& from_way, int shp_type) {
                if (!from_way.nodes().has_position()) {
                    throw std::runtime_error("node coordinates not available for building way geometry");
                }
                int size = from_way.nodes().size();
                if (size == 0 || size == 1) {
                    std::cerr << "error building way geometry for way " << from_way.id() << ": must at least contain two nodes" << std::endl;
                    throw Osmium::Geometry::IllegalGeometry();
                }

                std::vector<double> lon_checked;
                lon_checked.reserve(size);
                lon_checked.push_back(from_way[0].position().lon());

                std::vector<double> lat_checked;
                lat_checked.reserve(size);
                lat_checked.push_back(from_way[0].position().lat());

                for (int i=1; i < size; i++) {
                    if (from_way[i] == from_way[i-1]) {
                        std::cerr << "warning building way geometry for way " << from_way.id() << ": contains node " << from_way[i].ref() << " twice" << std::endl;
                    } else if (from_way[i].position() == from_way[i-1].position()) {
                        std::cerr << "warning building way geometry for way " << from_way.id() << ": contains position " << from_way[i].position() << " twice" << std::endl;
                    } else {
                        lon_checked.push_back(from_way[i].position().lon());
                        lat_checked.push_back(from_way[i].position().lat());
                    }
                }
                if (lon_checked.size() == 1) {
                    std::cerr << "error building way geometry for way " << from_way.id() << ": must at least contain two different points" << std::endl;
                    throw Osmium::Geometry::IllegalGeometry();
                }
                if (from_way.reverse()) {
                    std::reverse(lon_checked.begin(), lon_checked.end());
                    std::reverse(lat_checked.begin(), lat_checked.end());
                }
                return SHPCreateSimpleObject(shp_type, lon_checked.size(), &(lon_checked[0]), &(lat_checked[0]), NULL);
            }

        }

        /**
         * Create Shapelib geometry of a LineString.
         *
         * Caller takes ownership. You have to call
         * SHPDestroyObject() with this geometry when you are done.
         */
        inline SHPObject* create_shp_object(const Osmium::Geometry::LineString& linestring) {
            return create_line_or_polygon(linestring, SHPT_ARC);
        }

        /**
         * Create Shapelib geometry of a Polygon.
         *
         * Caller takes ownership. You have to call
         * SHPDestroyObject() with this geometry when you are done.
         */
        inline SHPObject* create_shp_object(const Osmium::Geometry::Polygon& polygon) {
            return create_line_or_polygon(polygon, SHPT_POLYGON);
        }

        namespace {

            inline void dump_geometry(const geos::geom::Geometry* g, std::vector<int>& part_start_list, std::vector<double>& x_list, std::vector<double>& y_list) {
                switch (g->getGeometryTypeId()) {
                    case geos::geom::GEOS_MULTIPOLYGON:
                    case geos::geom::GEOS_MULTILINESTRING: {
                        for (geos::geom::GeometryCollection::const_iterator it = dynamic_cast<const geos::geom::GeometryCollection*>(g)->begin();
                                it != dynamic_cast<const geos::geom::GeometryCollection*>(g)->end(); ++it) {
                            dump_geometry(*it, part_start_list, x_list, y_list);
                        }
                        break;
                    }
                    case geos::geom::GEOS_POLYGON: {
                        const geos::geom::Polygon* polygon = dynamic_cast<const geos::geom::Polygon*>(g);
                        dump_geometry(polygon->getExteriorRing(), part_start_list, x_list, y_list);
                        for (size_t i=0; i < polygon->getNumInteriorRing(); ++i) {
                            dump_geometry(polygon->getInteriorRingN(i), part_start_list, x_list, y_list);
                        }
                        break;
                    }
                    case geos::geom::GEOS_LINESTRING:
                    case geos::geom::GEOS_LINEARRING: {
                        part_start_list.push_back(x_list.size());
                        const geos::geom::CoordinateSequence* cs = dynamic_cast<const geos::geom::LineString*>(g)->getCoordinatesRO();
                        for (size_t i = 0; i < cs->getSize(); ++i) {
                            x_list.push_back(cs->getX(i));
                            y_list.push_back(cs->getY(i));
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error("invalid geometry type encountered");
                }
            }

        }

        /**
         * Create Shapelib geometry of a MultiPolygon.
         *
         * Caller takes ownership. You have to call
         * SHPDestroyObject() with this geometry when you are done.
         */
        inline SHPObject* create_shp_object(const Osmium::Geometry::MultiPolygon& multipolygon) {
            const geos::geom::MultiPolygon* geos_multipolygon = multipolygon.borrow_geos_geometry();

            std::vector<double> x_list;
            std::vector<double> y_list;
            std::vector<int> part_start_list;

            dump_geometry(geos_multipolygon, part_start_list, x_list, y_list);

            return SHPCreateObject(
                       SHPT_POLYGON,           // nSHPType
                       -1,                     // iShape
                       part_start_list.size(), // nParts
                       &part_start_list[0],    // panPartStart
                       NULL,                   // panPartType
                       x_list.size(),          // nVertices,
                       &x_list[0],             // padfX
                       &y_list[0],             // padfY
                       NULL,                   // padfZ
                       NULL);                  // padfM
        }

        /**
         * Create Shapelib geometry of a Geometry.
         *
         * Caller takes ownership. You have to call
         * SHPDestroyObject() with this geometry when you are done.
         */
        inline SHPObject* create_shp_object(const Osmium::Geometry::Geometry& geometry) {
            /* this is rather ugly code but we have to do this here because we can't make this
               free function into a member function where C++ would to the polymorphy thing for us. */
            const Osmium::Geometry::Point* point = dynamic_cast<const Osmium::Geometry::Point*>(&geometry);
            if (point) {
                return create_shp_object(*point);
            }
            const Osmium::Geometry::LineString* linestring = dynamic_cast<const Osmium::Geometry::LineString*>(&geometry);
            if (linestring) {
                return create_shp_object(*linestring);
            }
            const Osmium::Geometry::Polygon* polygon = dynamic_cast<const Osmium::Geometry::Polygon*>(&geometry);
            if (polygon) {
                return create_shp_object(*polygon);
            }
            const Osmium::Geometry::MultiPolygon* multipolygon = dynamic_cast<const Osmium::Geometry::MultiPolygon*>(&geometry);
            if (multipolygon) {
                return create_shp_object(*multipolygon);
            }
            return NULL;
        }

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_SHPLIB_HPP
