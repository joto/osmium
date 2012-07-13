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
                : FromWay(way_node_list, reverse, id) {
            }

            /**
             * Create LineString geometry from a list of nodes in a way.
             */
            LineString(const Osmium::OSM::Way& way, ///< Way this geometry should be created from
                       bool reverse=false)          ///< Create reverse geometry
                : FromWay(way.nodes(), reverse, way.id()) {
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                LonLatListWriter<Osmium::OSM::WayNode> writer(out);
                out << "LINESTRING(" << std::setprecision(10);
                if (m_reverse) {
                    for_each(m_way_node_list->rbegin(), m_way_node_list->rend(), writer);
                } else {
                    for_each(m_way_node_list->begin(), m_way_node_list->end(), writer);
                }
                return out << ")";
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                write_binary_wkb_header(out, with_srid, wkbLineString);
                write_binary<uint32_t>(out, m_way_node_list->size());
                if (m_reverse) {
                    for (Osmium::OSM::WayNodeList::const_reverse_iterator it = m_way_node_list->rbegin(); it != m_way_node_list->rend(); ++it) {
                        write_binary<double>(out, it->lon());
                        write_binary<double>(out, it->lat());
                    }
                } else {
                    for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); it != m_way_node_list->end(); ++it) {
                        write_binary<double>(out, it->lon());
                        write_binary<double>(out, it->lat());
                    }
                }
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                write_hex_wkb_header(out, with_srid, wkbLineString);
                write_hex<uint32_t>(out, m_way_node_list->size());
                if (m_reverse) {
                    for (Osmium::OSM::WayNodeList::const_reverse_iterator it = m_way_node_list->rbegin(); it != m_way_node_list->rend(); ++it) {
                        write_hex<double>(out, it->lon());
                        write_hex<double>(out, it->lat());
                    }
                } else {
                    for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); it != m_way_node_list->end(); ++it) {
                        write_hex<double>(out, it->lon());
                        write_hex<double>(out, it->lat());
                    }
                }
                return out;
            }

#ifdef OSMIUM_WITH_GEOS
            /**
             * Create GEOS geometry of this LineString.
             *
             * Caller takes ownership.
             */
            geos::geom::Geometry* create_geos_geometry() const {
                try {
                    std::vector<geos::geom::Coordinate>* c = new std::vector<geos::geom::Coordinate>;
                    if (m_reverse) {
                        for (Osmium::OSM::WayNodeList::const_reverse_iterator it = m_way_node_list->rbegin(); it != m_way_node_list->rend(); ++it) {
                            c->push_back(it->position());
                        }
                    } else {
                        for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); it != m_way_node_list->end(); ++it) {
                            c->push_back(it->position());
                        }
                    }
                    geos::geom::CoordinateSequence* cs = Osmium::Geometry::geos_geometry_factory()->getCoordinateSequenceFactory()->create(c);
                    return static_cast<geos::geom::Geometry*>(Osmium::Geometry::geos_geometry_factory()->createLineString(cs));
                } catch (const geos::util::GEOSException& exc) {
                    if (Osmium::debug()) {
                        std::cerr << "error building geometry for way #" << id() <<
                                  " (returning NULL): " << exc.what();
                    }
                    return NULL;
                }
            }
#endif // OSMIUM_WITH_GEOS

#ifdef OSMIUM_WITH_OGR
            /**
             * Create OGR geometry of this LineString;
             *
             * Caller takes ownership.
             */
            OGRLineString* create_ogr_geometry() const {
                OGRLineString* p = new OGRLineString();
                if (m_reverse) {
                    for (Osmium::OSM::WayNodeList::const_reverse_iterator it = m_way_node_list->rbegin(); it != m_way_node_list->rend(); ++it) {
                        p->addPoint(it->lon(), it->lat());
                    }
                } else {
                    for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); it != m_way_node_list->end(); ++it) {
                        p->addPoint(it->lon(), it->lat());
                    }
                }
                return p;
            }
#endif // OSMIUM_WITH_OGR

        }; // class LineString

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_LINESTRING_HPP
