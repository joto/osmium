#ifndef OSMIUM_OUTPUT_OSM_HPP
#define OSMIUM_OUTPUT_OSM_HPP

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

namespace Osmium {

    namespace Output {

        namespace OSM {

            class Base {

            protected:

                OSMFile m_file;

                int get_fd() {
                    return m_file.get_fd();
                }

            public:

                Base(OSMFile& file) : m_file(file) {
                    m_file.open_for_output();
                }

                virtual ~Base() {
                }

                virtual void write_init() = 0;
                virtual void write_bounds(double minlon, double minlat, double maxlon, double maxlat) = 0;
                virtual void write(Osmium::OSM::Node *) = 0;
                virtual void write(Osmium::OSM::Way *) = 0;
                virtual void write(Osmium::OSM::Relation *) = 0;
                virtual void write_final() = 0;

            }; // class Base

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#include <osmium/output/osm/pbf.hpp>
#ifdef OSMIUM_WITH_OUTPUT_OSM_XML
# include <osmium/output/osm/xml.hpp>
#endif

#endif // OSMIUM_OUTPUT_OSM_HPP
