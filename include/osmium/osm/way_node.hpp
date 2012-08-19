#ifndef OSMIUM_OSM_WAY_NODE_HPP
#define OSMIUM_OSM_WAY_NODE_HPP

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

#include <osmium/osm/types.hpp>
#include <osmium/osm/position.hpp>

namespace Osmium {

    namespace OSM {

        class WayNode {

        public:

            WayNode(osm_object_id_t ref=0, const Position& position=Position()) :
                m_ref(ref),
                m_position(position) {
            }

            osm_object_id_t ref() const {
                return m_ref;
            }

            WayNode& ref(osm_object_id_t ref) {
                m_ref = ref;
                return *this;
            }

            const Position& position() const {
                return m_position;
            }

            Position& position() {
                return m_position;
            }

            WayNode& position(const Position& position) {
                m_position = position;
                return *this;
            }

            bool has_position() const {
                return m_position.defined();
            }

            double lon() const {
                return m_position.lon();
            }

            double lat() const {
                return m_position.lat();
            }

        private:

            osm_object_id_t m_ref;
            Position m_position;

        }; // class WayNode

        inline bool operator<(const WayNode& wn1, const WayNode& wn2) {
            return wn1.ref() < wn2.ref();
        }

        inline bool operator==(const WayNode& wn1, const WayNode& wn2) {
            return wn1.ref() == wn2.ref();
        }

        inline bool operator!=(const WayNode& wn1, const WayNode& wn2) {
            return !(wn1 == wn2);
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_WAY_NODE_HPP
