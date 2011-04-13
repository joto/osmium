#ifndef OSMIUM_HANDLER_FIND_BBOX_HPP
#define OSMIUM_HANDLER_FIND_BBOX_HPP

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

    namespace Handler {

        class FindBbox : public Base {

            double minlon, maxlon, minlat, maxlat;

        public:

            FindBbox() : Base() {
                minlon =  1000;
                maxlon = -1000;
                minlat =  1000;
                maxlat = -1000;
            }

            double get_minlon() {
                return minlon;
            }

            double get_maxlon() {
                return maxlon;
            }

            double get_minlat() {
                return minlat;
            }

            double get_maxlat() {
                return maxlat;
            }

            void callback_node(OSM::Node *node) {
                if (node->get_lon() < minlon) minlon = node->get_lon();
                if (node->get_lon() > maxlon) maxlon = node->get_lon();
                if (node->get_lat() < minlat) minlat = node->get_lat();
                if (node->get_lat() > maxlat) maxlat = node->get_lat();
            }

            void callback_after_nodes() const {
                throw Osmium::Input::StopReading();
            }

        }; // class FindBbox

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_FIND_BBOX_HPP
