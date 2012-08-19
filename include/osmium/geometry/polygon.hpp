#ifndef OSMIUM_GEOMETRY_POLYGON_HPP
#define OSMIUM_GEOMETRY_POLYGON_HPP

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

#include <algorithm>
#include <iomanip>
#include <ostream>

#include <osmium/geometry/from_way.hpp>

namespace Osmium {

    namespace Geometry {

        /**
         * Polygon geometry.
         */
        class Polygon : public FromWay {

        public:

            /**
             * Create Polygon geometry from a list of nodes.
             */
            Polygon(const Osmium::OSM::WayNodeList& way_node_list,
                    bool reverse=false,
                    osm_object_id_t id=0) :
                FromWay(way_node_list, reverse, id) {
                if (!way_node_list.is_closed()) {
                    throw RingNotClosed();
                }
            }

            /**
             * Create Polygon geometry from a list of nodes in a way.
             */
            Polygon(const Osmium::OSM::Way& way, bool reverse=false) :
                FromWay(way.nodes(), reverse, way.id()) {
                if (!way.nodes().is_closed()) {
                    throw RingNotClosed();
                }
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                LonLatListWriter<Osmium::OSM::WayNode> writer(out);
                out << "POLYGON((" << std::setprecision(10);
                for_each(nodes().begin(), nodes().end(), writer);
                return out << "))";
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                write_binary_wkb_header(out, with_srid, wkbPolygon);
                write_binary<uint32_t>(out, 1); // ring count
                write_binary<uint32_t>(out, nodes().size()); // ring #1 point count
                for (Osmium::OSM::WayNodeList::const_iterator it = nodes().begin(); it != nodes().end(); ++it) {
                    write_binary<double>(out, it->lon());
                    write_binary<double>(out, it->lat());
                }
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                write_hex_wkb_header(out, with_srid, wkbPolygon);
                write_hex<uint32_t>(out, 1); // ring count
                write_hex<uint32_t>(out, nodes().size()); // ring #1 point count
                for (Osmium::OSM::WayNodeList::const_iterator it = nodes().begin(); it != nodes().end(); ++it) {
                    write_hex<double>(out, it->lon());
                    write_hex<double>(out, it->lat());
                }
                return out;
            }

        }; // class Polygon

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_POLYGON_HPP
