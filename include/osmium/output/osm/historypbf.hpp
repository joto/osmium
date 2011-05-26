#ifndef OSMIUM_OUTPUT_OSM_PBFHISTORY_HPP
#define OSMIUM_OUTPUT_OSM_PBFHISTORY_HPP

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

            class HistoryPBF : public PBF {

            public:

                HistoryPBF() : PBF() {
                    is_history_file(true);
                }

                HistoryPBF(std::string &filename) : PBF(filename) {
                    is_history_file(true);
                }

                ~HistoryPBF() {
                }

            }; // class HistoryPBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBFHISTORY_HPP
