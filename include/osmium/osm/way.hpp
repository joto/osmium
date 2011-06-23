#ifndef OSMIUM_OSM_WAY_HPP
#define OSMIUM_OSM_WAY_HPP

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

#include <stdexcept>
#include <iostream>

#include <osmium/osm/way_node_list.hpp>

#ifdef OSMIUM_WITH_SHPLIB
# include <shapefil.h>
# include <algorithm>
#endif // OSMIUM_WITH_SHPLIB

/** @file
*   @brief Contains the Osmium::OSM::Way class.
*/

#ifdef OSMIUM_WITH_GEOS
# include <geos/geom/Coordinate.h>
# include <geos/geom/CoordinateSequenceFactory.h>
# include <geos/geom/Geometry.h>
# include <geos/geom/Point.h>
# include <geos/util/GEOSException.h>
#endif // OSMIUM_WITH_GEOS

namespace Osmium {

    namespace OSM {

        class Way : public Object {

            WayNodeList m_node_list;

        public:

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_nodes_instance;
            v8::Local<v8::Object> js_geom_instance;
#endif // OSMIUM_WITH_JAVASCRIPT

            /// Construct a Way object.
            Way() : Object(), m_node_list() {
                init();
                reset();
            }

            Way(int size_of_node_list) : Object(), m_node_list(size_of_node_list) {
                init();
                reset();
            }

            /// Copy a Way object.
            Way(const Way& w) : Object(w) {
                init();
                m_node_list = w.m_node_list;
            }

            const WayNodeList& way_node_list() const {
                return m_node_list;
            }

        private:

            void init() {
#ifdef OSMIUM_WITH_JAVASCRIPT
                js_object_instance = JavascriptTemplate::get<JavascriptTemplate>().create_instance(this);
                js_geom_instance   = Osmium::Javascript::Template::create_way_geom_instance(this);
#endif // OSMIUM_WITH_JAVASCRIPT
            }

        public:

            osm_object_type_t get_type() const {
                return WAY;
            }

            void reset() {
                Object::reset();
                m_node_list.clear();
            }

            osm_object_id_t get_node_id(osm_sequence_id_t n) const {
                return m_node_list[n].ref();
            }

            double get_lon(osm_sequence_id_t n) const {
                return m_node_list[n].position().lon();
            }

            double get_lat(osm_sequence_id_t n) const {
                return m_node_list[n].position().lat();
            }

            /**
            * Add a node with the given id to the way.
            *
            * Will throw a range error if the way already has max_nodes_in_way nodes.
            */
            void add_node(osm_object_id_t ref) {
                m_node_list.add(ref);
            }

            /**
            * Returns the number of nodes in this way.
            */
            osm_sequence_id_t node_count() const {
                return m_node_list.size();
            }

            /**
             * Returns the id of the first node.
             */
            osm_object_id_t get_first_node_id() const {
                return m_node_list.front().ref();
            }

            /**
             * Returns the id of the last node.
             */
            osm_object_id_t get_last_node_id() const {
                return m_node_list.back().ref();
            }

#ifdef OSMIUM_WITH_GEOS
            /**
             * Returns the GEOS geometry of the first node.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Point *get_first_node_geometry() const {
                if (!m_node_list.front().has_position()) {
                    throw std::range_error("geometry for nodes not available");
                }
                return Osmium::global.geos_geometry_factory->createPoint(m_node_list.front().position());
            }

            /**
             * Returns the GEOS geometry of the last node.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Point *get_last_node_geometry() const {
                if (!m_node_list.back().has_position()) {
                    throw std::range_error("geometry for nodes not available");
                }
                return Osmium::global.geos_geometry_factory->createPoint(m_node_list.back().position());
            }
#endif // OSMIUM_WITH_GEOS

            /**
            * Check whether this way is closed. A way is closed if the first and last node have the same id.
            */
            bool is_closed() const {
                return m_node_list.is_closed();
            }

            /**
            * Set coordinates for the nth node in this way.
            */
            void set_node_coordinates(osm_sequence_id_t n, double nlon, double nlat) {
                if (n >= m_node_list.size()) {
                    throw std::range_error("trying to set coordinate for unknown node");
                }
                m_node_list[n].position(Position(nlon, nlat));
            }

            /**
             * Are the node coordinates set for this way?
             */
            bool node_coordinates_set() const {
                return m_node_list.has_position();
            }

#ifdef OSMIUM_WITH_GEOS
            /**
             * Returns the GEOS geometry of the way.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Geometry *create_geos_geometry() const {
                try {
                    std::vector<geos::geom::Coordinate> *c = new std::vector<geos::geom::Coordinate>;
                    for (osm_sequence_id_t i=0; i < m_node_list.size(); i++) {
                        c->push_back(m_node_list[i].position());
                    }
                    geos::geom::CoordinateSequence *cs = Osmium::global.geos_geometry_factory->getCoordinateSequenceFactory()->create(c);
                    return (geos::geom::Geometry *) Osmium::global.geos_geometry_factory->createLineString(cs);
                } catch (const geos::util::GEOSException& exc) {
                    std::cerr << "error building way geometry, leave it as NULL" << std::endl;
                    return NULL;
                }
            }
#endif // OSMIUM_WITH_GEOS


#ifdef OSMIUM_WITH_SHPLIB
            /**
            * Create a SHPObject for this way and return it. You have to call
            * SHPDestroyObject() with this object when you are done.
            */
            SHPObject *create_shpobject(int shp_type, bool reverse_way) {
                if (!m_node_list.has_position()) {
                    throw std::runtime_error("node coordinates not available for building way geometry");
                }
                int size = m_node_list.size();
                if (size == 0 || size == 1) {
                    if (Osmium::global.debug) std::cerr << "error building way geometry for way " << get_id() << ": must at least contain two nodes" << std::endl;
                    throw Osmium::Exception::IllegalGeometry();
                }

                std::vector<double> lon_checked;
                lon_checked.reserve(size);
                lon_checked.push_back(m_node_list[0].position().lon());

                std::vector<double> lat_checked;
                lat_checked.reserve(size);
                lat_checked.push_back(m_node_list[0].position().lat());

                for (int i=1; i < size; i++) {
                    if (m_node_list[i] == m_node_list[i-1]) {
                        if (Osmium::global.debug) std::cerr << "warning building way geometry for way " << get_id() << ": contains node " << m_node_list[i].ref() << " twice" << std::endl;
                    } else if (m_node_list[i].position() == m_node_list[i-1].position()) {
                        if (Osmium::global.debug) std::cerr << "warning building way geometry for way " << get_id() << ": contains position " << m_node_list[i].position() << " twice" << std::endl;
                    } else {
                        lon_checked.push_back(m_node_list[i].position().lon());
                        lat_checked.push_back(m_node_list[i].position().lat());
                    }
                }
                if (lon_checked.size() == 1) {
                    if (Osmium::global.debug) std::cerr << "error building way geometry for way " << get_id() << ": must at least contain two different points" << std::endl;
                    throw Osmium::Exception::IllegalGeometry();
                }
                if (reverse_way) {
                    reverse(lon_checked.begin(), lon_checked.end());
                    reverse(lat_checked.begin(), lat_checked.end());
                }
                return SHPCreateSimpleObject(shp_type, lon_checked.size(), &(lon_checked[0]), &(lat_checked[0]), NULL);
            }

            SHPObject *create_shp_line(std::string& transformation) {
                return create_shpobject(SHPT_ARC, transformation == "reverse" ? 1 : 0);
            }

            SHPObject *create_shp_polygon(std::string& transformation) {
                return create_shpobject(SHPT_POLYGON, transformation == "reverse" ? 1 : 0);
            }
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Handle<v8::Value> js_get_nodes() const {
                return m_node_list.js_instance();
            }

            v8::Handle<v8::Value> js_get_geom() const {
                return js_geom_instance;
            }

            std::ostream& geom_as_wkt(std::ostream& s) const {
                s << "LINESTRING(";
                for (osm_sequence_id_t i=0; i < m_node_list.size(); i++) {
                    if (i != 0) {
                        s << ',';
                    }
                    s << m_node_list[i].position().lon()
                      << ' '
                      << m_node_list[i].position().lat();
                }
                return s << ')';
            }

            v8::Handle<v8::Value> js_get_geom_property(v8::Local<v8::String> property) const {
                v8::String::Utf8Value key(property);

                if (!node_coordinates_set()) {
                    return v8::Undefined();
                }

                if (!strcmp(*key, "as_wkt")) {
                    std::ostringstream oss;
                    geom_as_wkt(oss);
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_ewkt")) {
                    std::ostringstream oss;
                    oss << "SRID=4326;";
                    geom_as_wkt(oss);
                    return v8::String::New(oss.str().c_str());
                } else if (!strcmp(*key, "as_array")) {
                    v8::Local<v8::Array> linestring = v8::Array::New(m_node_list.size());
                    for (osm_sequence_id_t i=0; i < m_node_list.size(); i++) {
                        v8::Local<v8::Array> coord = v8::Array::New(2);
                        coord->Set(v8::Integer::New(0), v8::Number::New(m_node_list[i].position().lon()));
                        coord->Set(v8::Integer::New(1), v8::Number::New(m_node_list[i].position().lat()));
                        linestring->Set(v8::Integer::New(i), coord);
                    }
                    return linestring;
                } else if (!strcmp(*key, "as_polygon_array") && is_closed()) {
                    v8::Local<v8::Array> polygon = v8::Array::New(1);
                    v8::Local<v8::Array> ring = v8::Array::New(m_node_list.size());
                    for (osm_sequence_id_t i=0; i < m_node_list.size(); i++) {
                        v8::Local<v8::Array> coord = v8::Array::New(2);
                        coord->Set(v8::Integer::New(0), v8::Number::New(m_node_list[i].position().lon()));
                        coord->Set(v8::Integer::New(1), v8::Number::New(m_node_list[i].position().lat()));
                        ring->Set(v8::Integer::New(i), coord);
                    }
                    polygon->Set(v8::Integer::New(0), ring);
                    return polygon;
                } else {
                    return v8::Undefined();
                }
            }

            struct JavascriptTemplate : public Osmium::OSM::Object::JavascriptTemplate {

                JavascriptTemplate() : Osmium::OSM::Object::JavascriptTemplate() {
                    js_template->SetAccessor(v8::String::New("nodes"), accessor_getter<Way, &Way::js_get_nodes>);
                    js_template->SetAccessor(v8::String::New("geom"),  accessor_getter<Way, &Way::js_get_geom>);
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        }; // class Way

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_WAY_HPP
