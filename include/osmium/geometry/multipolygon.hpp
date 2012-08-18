#ifndef OSMIUM_GEOMETRY_MULTIPOLYGON_HPP
#define OSMIUM_GEOMETRY_MULTIPOLYGON_HPP

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

#include <geos/geom/Geometry.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTWriter.h>

#include <osmium/smart_ptr.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/geometry.hpp>
#include <osmium/geometry/polygon.hpp>
#include <osmium/geometry/geos.hpp>

namespace Osmium {

    namespace Geometry {

        class MultiPolygon : public Geometry {

        public:

            MultiPolygon(const Osmium::OSM::Area& area) :
                Geometry(area.id()),
                m_area(area) {

                // if the area doesn't have a geometry yet it means it was created from a way
                // and we create the geometry from the node list.
                if (!area.geos_geometry()) {
                    Osmium::Geometry::Polygon polygon(area.m_node_list, false, area.id());
                    std::vector<geos::geom::Geometry*>* geos_polygons = new std::vector<geos::geom::Geometry*>(1, Osmium::Geometry::create_geos_geometry(polygon));
                    geos::geom::MultiPolygon* geos_multipolygon = Osmium::Geometry::geos_geometry_factory()->createMultiPolygon(geos_polygons);
                    assert(geos_multipolygon);
                    m_area.geos_geometry(geos_multipolygon);
                }
            }

            ~MultiPolygon() {
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                geos::io::WKTWriter writer;
                // XXX only available in later GEOS versions
//                writer.setRoundingPrecision(10);
                return out << writer.write(borrow_geos_geometry());
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.write(*borrow_geos_geometry(), out);
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.writeHEX(*borrow_geos_geometry(), out);
                return out;
            }

            /**
             * Get GEOS MultiPolygon geometry. The geometry still
             * belongs to this object, you are not allowed to change
             * or free it.
             */
            const geos::geom::MultiPolygon* borrow_geos_geometry() const {
                if (!m_area.geos_geometry()) {
                    throw Osmium::Geometry::NoGeometry();
                }

                return m_area.geos_geometry();
            }

        private:

            const Osmium::OSM::Area& m_area;

        }; // class MultiPolygon

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_MULTIPOLYGON_HPP
