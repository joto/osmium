#ifndef OSMIUM_GEOMETRY_LINESTRING_HPP
#define OSMIUM_GEOMETRY_LINESTRING_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

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

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
#endif // OSMIUM_WITH_SHPLIB

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
                    for (Osmium::OSM::WayNodeList::const_reverse_iterator it = m_way_node_list->rbegin(); 
                        it != m_way_node_list->rend(); it++) {
                        write_binary<double>(out, it->lon());
                        write_binary<double>(out, it->lat());
                    }
                } else {
                    for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); 
                        it != m_way_node_list->end(); it++) {
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
                    for (Osmium::OSM::WayNodeList::const_reverse_iterator it = m_way_node_list->rbegin(); 
                        it != m_way_node_list->rend(); it++) {
                        write_hex<double>(out, it->lon());
                        write_hex<double>(out, it->lat());
                    }
                } else {
                    for (Osmium::OSM::WayNodeList::const_iterator it = m_way_node_list->begin(); 
                        it != m_way_node_list->end(); it++) {
                        write_hex<double>(out, it->lon());
                        write_hex<double>(out, it->lat());
                    }
                }
                return out;
            }

#ifdef OSMIUM_WITH_GEOS
            /**
             * Returns the GEOS geometry of the way.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Geometry *create_geos_geometry() const {
                try {
                    std::vector<geos::geom::Coordinate>* c = new std::vector<geos::geom::Coordinate>;
                    if (m_reverse) {
                        for (osm_sequence_id_t i=m_way_node_list->size(); i > 0; --i) {
                            c->push_back((*m_way_node_list)[i].position());
                        }
                    } else {
                        for (osm_sequence_id_t i=0; i < m_way_node_list->size(); ++i) {
                            c->push_back((*m_way_node_list)[i].position());
                        }
                    }
                    geos::geom::CoordinateSequence *cs = Osmium::Geometry::geos_geometry_factory()->getCoordinateSequenceFactory()->create(c);
                    return (geos::geom::Geometry *) Osmium::Geometry::geos_geometry_factory()->createLineString(cs);
                } catch (const geos::util::GEOSException& exc) {
                    if (Osmium::debug()) {
                        std::cerr << "error building geometry for way #" << m_id << 
                            " (returning NULL): " << exc.what();
                    }
                    return NULL;
                }
            }
#endif // OSMIUM_WITH_GEOS

#ifdef OSMIUM_WITH_SHPLIB
            SHPObject *create_shp_object() const {
                return create_line_or_polygon(SHPT_ARC);
            }
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_instance() const {
                return JavascriptTemplate::get<JavascriptTemplate>().create_instance((void *)this);
            }

            v8::Handle<v8::Value> js_to_array(const v8::Arguments& /*args*/) {
                v8::HandleScope scope;
                v8::Local<v8::Array> linestring = v8::Array::New(m_way_node_list->size());
                unsigned int max = m_way_node_list->size() - 1;
                if (m_reverse) {
                    for (unsigned int i=0; i <= max; ++i) {
                        linestring->Set(max - i, (*m_way_node_list)[i].position().js_to_array());
                    }
                } else {
                    for (unsigned int i=0; i <= max; ++i) {
                        linestring->Set(i, (*m_way_node_list)[i].position().js_to_array());
                    }
                }
                return scope.Close(linestring);
            }

            struct JavascriptTemplate : public Osmium::Geometry::Geometry::JavascriptTemplate {

                JavascriptTemplate() : Osmium::Geometry::Geometry::JavascriptTemplate() {
                    js_template->Set("toArray",  v8::FunctionTemplate::New(function_template<LineString, &LineString::js_to_array>));
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        }; // class LineString

    } // namespace Geometry

} // namespace Osmium

#ifdef OSMIUM_WITH_JAVASCRIPT
v8::Handle<v8::Value> Osmium::OSM::Way::js_geom() const {
    if (m_node_list.has_position()) {
        Osmium::Geometry::LineString* geom = new Osmium::Geometry::LineString(*this);
        return Osmium::Javascript::Template::get<Osmium::Geometry::LineString::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::LineString>(geom);
    } else {
        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
        return Osmium::Javascript::Template::get<Osmium::Geometry::Null::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Null>(geom);
    }
}

v8::Handle<v8::Value> Osmium::OSM::Way::js_reverse_geom() const {
    if (m_node_list.has_position()) {
        Osmium::Geometry::LineString* geom = new Osmium::Geometry::LineString(*this, true);
        return Osmium::Javascript::Template::get<Osmium::Geometry::LineString::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::LineString>(geom);
    } else {
        Osmium::Geometry::Null* geom = new Osmium::Geometry::Null();
        return Osmium::Javascript::Template::get<Osmium::Geometry::Null::JavascriptTemplate>().create_persistent_instance<Osmium::Geometry::Null>(geom);
    }
}
#endif // OSMIUM_WITH_JAVASCRIPT

#endif // OSMIUM_GEOMETRY_LINESTRING_HPP
