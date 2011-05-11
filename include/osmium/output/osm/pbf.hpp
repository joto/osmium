#ifndef OSMIUM_OUTPUT_OSM_PBF_HPP
#define OSMIUM_OUTPUT_OSM_PBF_HPP

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

    namespace Output {

        namespace OSM {

            class PBF : public Base {

            public:

                PBF() : Base() {
                    throw std::runtime_error("pbf writer not implemented yet");
                }

                PBF(std::string &filename) : Base() {
                    throw std::runtime_error("pbf writer not implemented yet");
                }

                void write_init() {
                }

                void write_bounds(double minlon, double minlat, double maxlon, double maxlat) {
                }

                void write(Osmium::OSM::Node *node) {
                }

                void write(Osmium::OSM::Way *way) {
                }

                void write(Osmium::OSM::Relation *relation) {
                }

                void write_final() {
                    // sth. useful
                }

            }; // class PBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBF_HPP
