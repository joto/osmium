#ifndef OSMIUM_OSM_META_HPP
#define OSMIUM_OSM_META_HPP

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

#include <osmium/osm/bounds.hpp>

namespace Osmium {

    namespace OSM {

        /**
         * Meta information from the header of an OSM file.
         */
        class Meta {

        public:

            Meta() : m_bounds() {
            }

            Meta(const Bounds& bounds) : m_bounds(bounds) {
            }

            Bounds& bounds() {
                return m_bounds;
            }

            const Bounds& bounds() const {
                return m_bounds;
            }

        private:

            Bounds m_bounds;

        }; // class Meta

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_META_HPP
