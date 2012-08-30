#ifndef OSMIUM_GEOMETRY_LINESTRING_HPP
#define OSMIUM_GEOMETRY_LINESTRING_HPP

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

#include <osmium/geometry/from_way.hpp>

namespace Osmium {

    namespace Geometry {

        /**
         * LineString geometry.
         */
        class LineString : public FromWay {

        public:

            /**
             * Create LineString geometry from a list of nodes.
             */
            LineString(const Osmium::OSM::WayNodeList& way_node_list, ///< Way node list this geometry should be created from
                       bool reverse=false,                            ///< Create reverse geometry
                       osm_object_id_t id=0)                          ///< Object ID of the way this geometry was created from
                :
                FromWay(way_node_list, reverse, id) {
            }

            /**
             * Create LineString geometry from a list of nodes in a way.
             */
            LineString(const Osmium::OSM::Way& way, ///< Way this geometry should be created from
                       bool reverse=false)          ///< Create reverse geometry
                :
                FromWay(way.nodes(), reverse, way.id()) {
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                LonLatListWriter<Osmium::OSM::WayNode> writer(out);
                out << "LINESTRING(" << std::setprecision(10);
                if (reverse()) {
                    for_each(nodes().rbegin(), nodes().rend(), writer);
                } else {
                    for_each(nodes().begin(), nodes().end(), writer);
                }
                return out << ")";
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                write_binary_wkb_header(out, with_srid, wkbLineString);
                write_binary<uint32_t>(out, nodes().size());
                if (reverse()) {
                    for (Osmium::OSM::WayNodeList::const_reverse_iterator it = nodes().rbegin(); it != nodes().rend(); ++it) {
                        write_binary<double>(out, it->lon());
                        write_binary<double>(out, it->lat());
                    }
                } else {
                    for (Osmium::OSM::WayNodeList::const_iterator it = nodes().begin(); it != nodes().end(); ++it) {
                        write_binary<double>(out, it->lon());
                        write_binary<double>(out, it->lat());
                    }
                }
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                write_hex_wkb_header(out, with_srid, wkbLineString);
                write_hex<uint32_t>(out, nodes().size());
                if (reverse()) {
                    for (Osmium::OSM::WayNodeList::const_reverse_iterator it = nodes().rbegin(); it != nodes().rend(); ++it) {
                        write_hex<double>(out, it->lon());
                        write_hex<double>(out, it->lat());
                    }
                } else {
                    for (Osmium::OSM::WayNodeList::const_iterator it = nodes().begin(); it != nodes().end(); ++it) {
                        write_hex<double>(out, it->lon());
                        write_hex<double>(out, it->lat());
                    }
                }
                return out;
            }

        }; // class LineString

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_LINESTRING_HPP
