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

#ifdef OSMIUM_WITH_GEOS
# include <geos/io/WKBWriter.h>
#endif // OSMIUM_WITH_GEOS

#include <osmium/osm/area.hpp>
#include <osmium/geometry.hpp>

namespace Osmium {

    namespace Geometry {

        class MultiPolygon : public Geometry {

        public:

            MultiPolygon(const Osmium::OSM::Area& area) : Geometry(area.id()), m_area(&area) {
#ifdef OSMIUM_WITH_GEOS
                if (!m_area->get_geometry()) {
                    throw Osmium::Exception::IllegalGeometry();
                }
#endif // OSMIUM_WITH_GEOS
            }

            const Osmium::OSM::Area* area() const {
                return m_area;
            }

#ifdef OSMIUM_WITH_GEOS

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                geos::io::WKTWriter writer;
                return out << writer.write(m_area->get_geometry());
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.write(*(m_area->get_geometry()), out);
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                geos::io::WKBWriter writer;
                writer.setIncludeSRID(with_srid);
                writer.writeHEX(*(m_area->get_geometry()), out);
                return out;
            }
#endif // OSMIUM_WITH_GEOS

        private:

            const Osmium::OSM::Area* m_area;

        }; // class MultiPolygon

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_MULTIPOLYGON_HPP
