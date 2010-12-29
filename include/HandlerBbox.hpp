#ifndef OSMIUM_HANDLER_BBOX_HPP
#define OSMIUM_HANDLER_BBOX_HPP

namespace Osmium {

    namespace Handler {

        class Bbox : public Base {

            double minlon, maxlon, minlat, maxlat;

          public:

            Bbox() {
                minlon =  1000;
                maxlon = -1000;
                minlat =  1000;
                maxlat = -1000;
            }

            void callback_node(OSM::Node *node) {
                if (node->get_lon() < minlon) minlon = node->get_lon();
                if (node->get_lon() > maxlon) maxlon = node->get_lon();
                if (node->get_lat() < minlat) minlat = node->get_lat();
                if (node->get_lat() > maxlat) maxlat = node->get_lat();
            }

            void callback_final() {
                std::cerr << "minlon=" << minlon << " maxlon=" << maxlon << " minlat=" << minlat << " maxlat=" << maxlat << std::endl;
            }

        }; // class Bbox

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_BBOX_HPP
