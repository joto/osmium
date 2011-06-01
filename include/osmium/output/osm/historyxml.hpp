#ifndef OSMIUM_OUTPUT_OSM_XMLHISTORY_HPP
#define OSMIUM_OUTPUT_OSM_XMLHISTORY_HPP

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

            class HistoryXML : public XML {

            public:

                HistoryXML() : XML() {
                    is_history_file(true);
                }

                HistoryXML(std::string &filename) : XML(filename) {
                    is_history_file(true);
                }

                ~HistoryXML() {
                }

            }; // class HistoryXML

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_XMLHISTORY_HPP
