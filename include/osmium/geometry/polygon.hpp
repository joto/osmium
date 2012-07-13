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

#include <iomanip>
#include <algorithm>

#include <osmium/geometry/from_way.hpp>
#include <osmium/exceptions.hpp>

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
                    osm_object_id_t id=0)
                : FromWay(way_node_list, reverse, id) {
                if (!way_node_list.is_closed()) {
                    throw Osmium::Exception::IllegalGeometry();
                }
            }

            /**
             * Create Polygon geometry from a list of nodes in a way.
             */
            Polygon(const Osmium::OSM::Way& way, bool reverse=false)
                : FromWay(way.nodes(), reverse, way.id()) {
                if (!way.nodes().is_closed()) {
                    throw Osmium::Exception::IllegalGeometry();
                }
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool with_srid=false) const {
                if (with_srid) {
                    out << "SRID=4326;";
                }
                LonLatListWriter<Osmium::OSM::WayNode> writer(out);
                out << "POLYGON((" << std::setprecision(10);
                for_each(m_way_node_list->begin(), m_way_node_list->end(), writer);
                return out << "))";
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool with_srid=false) const {
                write_binary_wkb_header(out, with_srid, wkbPolygon);
                write_binary<uint32_t>(out, 1); // ring count
                write_binary<uint32_t>(out, m_way_node_list->size()); // ring #1 point count
                for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); it != m_way_node_list->end(); ++it) {
                    write_binary<double>(out, it->lon());
                    write_binary<double>(out, it->lat());
                }
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const {
                write_hex_wkb_header(out, with_srid, wkbPolygon);
                write_hex<uint32_t>(out, 1); // ring count
                write_hex<uint32_t>(out, m_way_node_list->size()); // ring #1 point count
                for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); it != m_way_node_list->end(); ++it) {
                    write_hex<double>(out, it->lon());
                    write_hex<double>(out, it->lat());
                }
                return out;
            }

#ifdef OSMIUM_WITH_GEOS
            /**
             * Creates GEOS geometry of this Polygon.
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
                    geos::geom::LinearRing* lr = Osmium::Geometry::geos_geometry_factory()->createLinearRing(cs);
                    return static_cast<geos::geom::Geometry*>(Osmium::Geometry::geos_geometry_factory()->createPolygon(lr, NULL));
                } catch (const geos::util::GEOSException& exc) {
                    std::cerr << "error building polygon geometry, leave it as NULL\n";
                    return NULL;
                }
            }
#endif // OSMIUM_WITH_GEOS

#ifdef OSMIUM_WITH_OGR
            /**
             * Create OGR geometry of this Polygon.
             *
             * Caller takes ownership.
             */
            OGRPolygon* create_ogr_geometry() const {
                OGRPolygon* p = new OGRPolygon();
                OGRLinearRing* r = new OGRLinearRing();
                if (m_reverse) {
                    for (Osmium::OSM::WayNodeList::const_reverse_iterator it = m_way_node_list->rbegin(); it != m_way_node_list->rend(); ++it) {
                        r->addPoint(it->lon(), it->lat());
                    }
                } else {
                    for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); it != m_way_node_list->end(); ++it) {
                        r->addPoint(it->lon(), it->lat());
                    }
                }
                p->addRingDirectly(r);
                return p;
            }
#endif // OSMIUM_WITH_OGR

        }; // class Polygon

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_POLYGON_HPP
