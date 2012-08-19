#ifndef OSMIUM_GEOMETRY_FROM_WAY_HPP
#define OSMIUM_GEOMETRY_FROM_WAY_HPP

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

#include <osmium/geometry.hpp>
#include <osmium/osm/way.hpp>

namespace Osmium {

    namespace Geometry {

        class FromWay : public Geometry {

        public:

            const Osmium::OSM::WayNodeList& nodes() const {
                return m_way_node_list;
            }

            const Osmium::OSM::WayNode& operator[](int i) const {
                return m_way_node_list[i];
            }

            bool reverse() const {
                return m_reverse;
            }

        protected:

            FromWay(const Osmium::OSM::WayNodeList& way_node_list,
                    bool reverse=false,
                    osm_object_id_t id=0) :
                Geometry(id),
                m_way_node_list(way_node_list),
                m_reverse(reverse) {
            }

        private:

            const Osmium::OSM::WayNodeList& m_way_node_list;
            const bool m_reverse;

        }; // class FromWay

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_FROM_WAY_HPP
