#ifndef OSMIUM_HANDLER_FIND_BBOX_HPP
#define OSMIUM_HANDLER_FIND_BBOX_HPP

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

        }; // class FindBbox

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_FIND_BBOX_HPP
