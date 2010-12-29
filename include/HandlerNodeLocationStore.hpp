#ifndef OSMIUM_HANDLER_NODELOCATIONSTORE_HPP
#define OSMIUM_HANDLER_NODELOCATIONSTORE_HPP

#include <stdexcept>
#include <google/sparsetable>

namespace Osmium {

    namespace Handler {

        class NodeLocationStore : public Base {

            struct coordinates {
                int32_t x;
                int32_t y;
            };

        protected:

            /**
            * the node location store will add this number to all ids,
            * this way at least some negative ids will be properly stored
            */
            static const int negative_id_offset = 1024 * 1024;

            static int32_t double_to_fix(double c) {
                return c * 10000000;
            }

            static double fix_to_double(int32_t c) {
                return ((double)c) / 10000000;
            }

        public:

            NodeLocationStore(bool debug) : Base(debug) {
            }

            virtual void callback_node(const OSM::Node *object) = 0;
            virtual void callback_way(OSM::Way *object) = 0;

        }; // class NodeLocationStore

        // Caution: Node store is not initialized to some zero value.
        // You can not find out if a node coordinate was ever set!
        class NLS_Array : public NodeLocationStore {

            struct coordinates *coordinates;

        public:

            NLS_Array(bool debug) : NodeLocationStore(debug) {
                const int max_nodes = 1.2 * 1024 * 1024 * 1024; // XXX make configurable, or autosizing?
                coordinates = (struct coordinates *) malloc(sizeof(struct coordinates) * max_nodes);
                if (!coordinates) {
                    throw std::bad_alloc();
                }
            }

            ~NLS_Array() {
                free(coordinates);
            }

            void callback_node(const OSM::Node *object) {
                const osm_object_id_t id = object->get_id() + NodeLocationStore::negative_id_offset;
                coordinates[id].x = double_to_fix(object->get_lon());
                coordinates[id].y = double_to_fix(object->get_lat());
            }
            
            void callback_way(OSM::Way *object) {
                const osm_sequence_id_t num_nodes = object->node_count();
                for (osm_sequence_id_t i=0; i < num_nodes; i++) {
                    osm_object_id_t node_id = object->nodes[i] + NodeLocationStore::negative_id_offset;
                    object->set_node_coordinates(i, fix_to_double(coordinates[node_id].x), fix_to_double(coordinates[node_id].y));
                }
            }

        }; // class NLS_Array

        class NLS_Sparsetable : public NodeLocationStore {

            google::sparsetable<struct coordinates> nodes_table;
            osm_object_id_t max_id;

        public:

            NLS_Sparsetable(bool debug) : NodeLocationStore(debug) {
                max_id = 1000;
                nodes_table.resize(max_id+1);
            }

            void callback_node(const OSM::Node *object) {
                osm_object_id_t id = object->get_id() + NodeLocationStore::negative_id_offset;
                if (id > max_id) {
                    max_id = id;
                    nodes_table.resize(max_id + 1000); // XXX
                }
                struct coordinates c = {
                    double_to_fix(object->get_lon()),
                    double_to_fix(object->get_lat())
                };
                nodes_table[id] = c;
            }
            
            void callback_way(OSM::Way *object) {
                osm_sequence_id_t num_nodes = object->node_count();
                for (osm_sequence_id_t i=0; i < num_nodes; i++) {
                    struct coordinates c = nodes_table[object->nodes[i] + NodeLocationStore::negative_id_offset];
                    object->set_node_coordinates(i, fix_to_double(c.x), fix_to_double(c.y));
                }
            }

        }; // class NLS_Sparsetable

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_NODELOCATIONSTORE_HPP
