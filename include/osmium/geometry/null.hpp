#ifndef OSMIUM_GEOMETRY_NULL_HPP
#define OSMIUM_GEOMETRY_NULL_HPP

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

namespace Osmium {

    namespace Geometry {

        /**
         * The Null geometry is a placeholder, if no valid geometry could be
         * created.
         */
        class Null : public Geometry {

        public:

            Null() : Geometry() {
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKT, bool /*with_srid=false*/) const {
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsWKB, bool /*with_srid=false*/) const {
                return out;
            }

            std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool /*with_srid=false*/) const {
                return out;
            }

        }; // class Null

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_HPP
