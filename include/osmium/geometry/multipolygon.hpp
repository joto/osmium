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

#include <osmium/osm/area.hpp>
#include <osmium/geometry.hpp>
#include <osmium/geometry/polygon.hpp>

namespace Osmium {

    namespace Geometry {

        class MultiPolygon : public Geometry {

        public:

            MultiPolygon(const Osmium::OSM::Area& area) :
                Geometry(area.id()),
                m_way_node_list(area.nodes()),
                m_geos_geometry(area.geos_geometry()) {
                if (!m_geos_geometry) {
                    Osmium::Geometry::Polygon polygon(m_way_node_list, false, area.id());
                    m_geos_geometry = polygon.create_geos_geometry();
                }
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                geos::io::WKTWriter writer;
                return out << writer.write(m_geos_geometry);
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.write(*(m_geos_geometry), out);
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.writeHEX(*(m_geos_geometry), out);
                return out;
            }

            geos::geom::Geometry* geos_geometry() const {
                return m_geos_geometry;
            }

            const Osmium::OSM::WayNodeList& nodes() const {
                return m_way_node_list;
            }

        private:

            const Osmium::OSM::WayNodeList& m_way_node_list;
            geos::geom::Geometry* m_geos_geometry;

        }; // class MultiPolygon

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_MULTIPOLYGON_HPP
