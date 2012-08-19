#ifndef OSMIUM_HANDLER_FIND_BBOX_HPP
#define OSMIUM_HANDLER_FIND_BBOX_HPP

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

#include <osmium/handler.hpp>
#include <osmium/osm/bounds.hpp>

namespace Osmium {

    namespace Handler {

        class FindBbox : public Base {

        public:

            FindBbox() :
                Base(),
                m_bounds() {
            }

            const Osmium::OSM::Bounds bounds() const {
                return m_bounds;
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                m_bounds.extend(node->position());
            }

            void after_nodes() const {
                throw StopReading();
            }

        private:

            Osmium::OSM::Bounds m_bounds;

        }; // class FindBbox

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_FIND_BBOX_HPP
