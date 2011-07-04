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

#include <osmium/osm/object.hpp>
#include <osmium/osm/way_node_list.hpp>

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

#include <osmium/geometry.hpp>

namespace Osmium {

    namespace OSM {

        class Way : public Object {

            WayNodeList m_node_list;

        public:

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

            const WayNodeList& nodes() const {
                return m_node_list;
            }

            WayNodeList& nodes() {
                return m_node_list;
            }

        private:

            void init() {
#ifdef OSMIUM_WITH_JAVASCRIPT
                js_object_instance = JavascriptTemplate::get<JavascriptTemplate>().create_instance(this);
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

            /**
            * Check whether this way is closed. A way is closed if the first and last node have the same id.
            */
            bool is_closed() const {
                return m_node_list.is_closed();
            }

#ifdef OSMIUM_WITH_GEOS
            /**
             * Returns the GEOS geometry of the first node.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Point* get_first_node_geometry() const {
                if (!m_node_list.front().has_position()) {
                    throw std::range_error("geometry for nodes not available");
                }
                return Osmium::Geometry::geos_geometry_factory()->createPoint(m_node_list.front().position());
            }

            /**
             * Returns the GEOS geometry of the last node.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Point* get_last_node_geometry() const {
                if (!m_node_list.back().has_position()) {
                    throw std::range_error("geometry for nodes not available");
                }
                return Osmium::Geometry::geos_geometry_factory()->createPoint(m_node_list.back().position());
            }

            /**
             * Returns the GEOS geometry of the way.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Geometry* create_geos_geometry() const {
                try {
                    std::vector<geos::geom::Coordinate> *c = new std::vector<geos::geom::Coordinate>;
                    for (osm_sequence_id_t i=0; i < m_node_list.size(); ++i) {
                        c->push_back(m_node_list[i].position());
                    }
                    geos::geom::CoordinateSequence *cs = Osmium::Geometry::geos_geometry_factory()->getCoordinateSequenceFactory()->create(c);
                    return (geos::geom::Geometry *) Osmium::Geometry::geos_geometry_factory()->createLineString(cs);
                } catch (const geos::util::GEOSException& exc) {
                    std::cerr << "error building way geometry, leave it as NULL\n";
                    return NULL;
                }
            }
#endif // OSMIUM_WITH_GEOS

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Handle<v8::Value> js_nodes() const {
                return m_node_list.js_instance();
            }

            v8::Handle<v8::Value> js_geom() const;

            v8::Handle<v8::Value> js_reverse_geom() const;

            struct JavascriptTemplate : public Osmium::OSM::Object::JavascriptTemplate {

                JavascriptTemplate() : Osmium::OSM::Object::JavascriptTemplate() {
                    js_template->SetAccessor(v8::String::NewSymbol("nodes"),        accessor_getter<Way, &Way::js_nodes>);
                    js_template->SetAccessor(v8::String::NewSymbol("geom"),         accessor_getter<Way, &Way::js_geom>);
                    js_template->SetAccessor(v8::String::NewSymbol("reverse_geom"), accessor_getter<Way, &Way::js_reverse_geom>);
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        }; // class Way

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_WAY_HPP
