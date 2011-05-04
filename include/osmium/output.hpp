#ifndef OSMIUM_OUTPUT_HPP
#define OSMIUM_OUTPUT_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

namespace Osmium {

    /**
     * @brief Namespace for classes implementing file output.
     */
    namespace Output {

        /**
         * @brief base class of all osmfile writers
         */
        class Osmfile {

            public:

                bool writeVisibleAttr;

                virtual ~Osmfile() {}

                virtual void writeBounds(double minlat, double minlon, double maxlat, double maxlon) = 0;
                virtual void write(Osmium::OSM::Node* e) = 0;
                virtual void write(Osmium::OSM::Way* e) = 0;
                virtual void write(Osmium::OSM::Relation* e) = 0;
        };

    } // namespace Output

} // namespace Osmium

#include <osmium/output/csv.hpp>
#include <osmium/output/shapefile.hpp>
#include <osmium/output/xml.hpp>

#endif // OSMIUM_OUTPUT_HPP
