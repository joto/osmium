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

            const Osmium::OSM::WayNodeList* nodes() const {
                return m_way_node_list;
            }

            bool reverse() const {
                return m_reverse;
            } 

        protected:

            FromWay(const Osmium::OSM::WayNodeList& way_node_list,
                    bool reverse=false,
                    osm_object_id_t id=0)
                : Geometry(id),
                  m_way_node_list(&way_node_list),
                  m_reverse(reverse) {
            }

#ifdef OSMIUM_WITH_SHPLIB
            /**
             * Create a SHPObject for this way and return it.
             *
             * Caller takes ownership. You have to call
             * SHPDestroyObject() with this geometry when you are done.
             */
            SHPObject* create_line_or_polygon(int shp_type) const {
                if (!m_way_node_list->has_position()) {
                    throw std::runtime_error("node coordinates not available for building way geometry");
                }
                int size = m_way_node_list->size();
                if (size == 0 || size == 1) {
                    if (Osmium::debug()) {
                        std::cerr << "error building way geometry for way " << id() << ": must at least contain two nodes" << std::endl;
                    }
                    throw Osmium::Exception::IllegalGeometry();
                }

                std::vector<double> lon_checked;
                lon_checked.reserve(size);
                lon_checked.push_back((*m_way_node_list)[0].position().lon());

                std::vector<double> lat_checked;
                lat_checked.reserve(size);
                lat_checked.push_back((*m_way_node_list)[0].position().lat());

                for (int i=1; i < size; i++) {
                    if ((*m_way_node_list)[i] == (*m_way_node_list)[i-1]) {
                        if (Osmium::debug()) {
                            std::cerr << "warning building way geometry for way " << id() << ": contains node " << (*m_way_node_list)[i].ref() << " twice" << std::endl;
                        }
                    } else if ((*m_way_node_list)[i].position() == (*m_way_node_list)[i-1].position()) {
                        if (Osmium::debug()) {
                            std::cerr << "warning building way geometry for way " << id() << ": contains position " << (*m_way_node_list)[i].position() << " twice" << std::endl;
                        }
                    } else {
                        lon_checked.push_back((*m_way_node_list)[i].position().lon());
                        lat_checked.push_back((*m_way_node_list)[i].position().lat());
                    }
                }
                if (lon_checked.size() == 1) {
                    if (Osmium::debug()) {
                        std::cerr << "error building way geometry for way " << id() << ": must at least contain two different points" << std::endl;
                    }
                    throw Osmium::Exception::IllegalGeometry();
                }
                if (m_reverse) {
                    std::reverse(lon_checked.begin(), lon_checked.end());
                    std::reverse(lat_checked.begin(), lat_checked.end());
                }
                return SHPCreateSimpleObject(shp_type, lon_checked.size(), &(lon_checked[0]), &(lat_checked[0]), NULL);
            }
#endif // OSMIUM_WITH_SHPLIB

            const Osmium::OSM::WayNodeList* m_way_node_list;
            const bool m_reverse;

        }; // class FromWay

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_FROM_WAY_HPP
