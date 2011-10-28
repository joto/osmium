#ifndef OSMIUM_OSM_NODE_HPP
#define OSMIUM_OSM_NODE_HPP

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

#include <boost/operators.hpp>

#include <osmium/osm/position.hpp>

/** @file
*   @brief Contains the Osmium::OSM::Node class.
*/

#include <osmium/osm/object.hpp>

namespace Osmium {

    namespace OSM {

        class Node : public Object, boost::less_than_comparable<Node> {

            static const int max_length_coordinate = 12 + 1; ///< maximum length of coordinate string (3 digits + dot + 8 digits + null byte)

            Position m_position;

        public:

            Node() : Object(), m_position() {
#ifdef OSMIUM_WITH_JAVASCRIPT
                js_object_instance = JavascriptTemplate::get<JavascriptTemplate>().create_instance(this);
#endif // OSMIUM_WITH_JAVASCRIPT
            }

            const Position position() const {
                return m_position;
            }

            Node& position(Position position) {
                m_position = position;
                return *this;
            }

            osm_object_type_t get_type() const {
                return NODE;
            }

            void set_x(double x) {
                m_position.lon(x);
            }

            void set_y(double y) {
                m_position.lat(y);
            }

            double get_lon() const {
                return m_position.lon();
            }

            double get_lat() const {
                return m_position.lat();
            }

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Handle<v8::Value> js_get_geom() const;

            struct JavascriptTemplate : public Osmium::OSM::Object::JavascriptTemplate {

                JavascriptTemplate() : Osmium::OSM::Object::JavascriptTemplate() {
                    js_template->SetAccessor(v8::String::NewSymbol("geom"), accessor_getter<Node, &Node::js_get_geom>);
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

            /**
             * Nodes can be ordered by id and version.
             * Note that we use the absolute value of the id for a
             * better ordering of objects with negative id.
             */
            friend bool operator<(const Node& lhs, const Node& rhs) {
                if (lhs.id() == rhs.id()) {
                    return lhs.version() < rhs.version();
                } else {
                    return abs(lhs.id()) < abs(rhs.id());
                }
            }

            /**
             * Ordering for shared_ptrs of Nodes.
             */
            friend bool operator<(const shared_ptr<Node const>& lhs, const shared_ptr<Node const>& rhs) {
                return *lhs < *rhs;
            }

        }; // class Node

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_NODE_HPP
