#ifndef OSMIUM_OSM_WAY_HPP
#define OSMIUM_OSM_WAY_HPP

#include <stdexcept>

namespace Osmium {

    namespace OSM {

        class Way : public Object {

            public:

            static const int max_nodes_in_way = 2000; //< there can be only 2000 nodes in a way as per OSM API 0.6 definition

            osm_sequence_id_t num_nodes;

            osm_object_id_t nodes[max_nodes_in_way];

            struct node_coordinates node_coordinates[max_nodes_in_way];

            Way() : Object() {
            }

            osm_object_type_t type() const {
                return WAY;
            }

            void reset() {
                Object::reset();
                num_nodes = 0;
            }

            void add_node(osm_object_id_t ref) {
                if (num_nodes < max_nodes_in_way) {
                    nodes[num_nodes++] = ref;
                }
                else {
                    throw std::range_error("no more than 2000 nodes in a way");
                }
            }

            osm_sequence_id_t node_count() const {
                return num_nodes;
            }

            bool is_closed() const {
                return nodes[0] == nodes[num_nodes-1];
            }

            void set_node_coordinates(osm_sequence_id_t node, double lon, double lat) {
                node_coordinates[node].lon = lon;
                node_coordinates[node].lat = lat;
            }

        }; // class Way

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_WAY_HPP
