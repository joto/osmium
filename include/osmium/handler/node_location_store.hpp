#ifndef OSMIUM_HANDLER_NODELOCATIONSTORE_HPP
#define OSMIUM_HANDLER_NODELOCATIONSTORE_HPP

#include <stdexcept>
#include <google/sparsetable>
#include <sys/mman.h>

namespace Osmium {

    namespace Handler {

        /**
        * Virtual base class for the different versions of the
        * node location store handlers. Use one of the child
        * classed depending on your needs.
        * Node locations are stored in 32 bit integers for the
        * x and y coordinates, respectively. This gives you an
        * accuracy of a few centimeters, good enough for OSM
        * use.
        */
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

            NodeLocationStore() : Base() {
            }

            virtual void callback_node(const OSM::Node *object) = 0;
            virtual void callback_after_nodes() = 0;
            virtual void callback_way(OSM::Way *object) = 0;

        }; // class NodeLocationStore

        /**
        * The NLS_Array node location store handler stores location
        * in a huge array. Currently the size of the array is hardcoded.
        * It must be large enough to hold all nodes. You'll need 8 bytes
        * for each node ID, currently thre are about 1 billion nodes IDs,
        * so you'll need about 8 GB of memory.
        *
        * Use this node location store if you are working with large
        * OSM files (like the whole planet or substantial extracts).
        *
        * Caution: The node store is not initialized to some zero value.
        * You can not find out if a node coordinate was ever set!
        */
        class NLS_Array : public NodeLocationStore {

            int max_nodes;
            struct coordinates *coordinates;

        public:

            NLS_Array() : NodeLocationStore() {
                max_nodes = 1.2 * 1024 * 1024 * 1024; // XXX make configurable, or autosizing?
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

            void callback_after_nodes() {
                if (Osmium::global.debug) {
                    std::cerr << "NodeLocationStore (Array) needs " << max_nodes << " * " << sizeof(struct coordinates) << "Bytes = " << max_nodes * sizeof(struct coordinates) / (1024 * 1204) << "MBytes" << std::endl;
                }
            }

            void callback_way(OSM::Way *object) {
                const osm_sequence_id_t num_nodes = object->node_count();
                for (osm_sequence_id_t i=0; i < num_nodes; i++) {
                    osm_object_id_t node_id = object->nodes[i] + NodeLocationStore::negative_id_offset;
                    object->set_node_coordinates(i, fix_to_double(coordinates[node_id].x), fix_to_double(coordinates[node_id].y));
                }
            }

        }; // class NLS_Array

        /**
        * The NLS_Sparsetable node location store handler stores location
        * in a sparsetable. The Google sparsetable is a data structure
        * that can hold sparsly filled tables in a very space efficient
        * way. It will resize automatically.
        *
        * Use this node location store if you are working with smaller
        * OSM files (like extracts if smaller countries).
        */
        class NLS_Sparsetable : public NodeLocationStore {

            google::sparsetable<struct coordinates> nodes_table;
            osm_object_id_t max_id;

        public:

            NLS_Sparsetable() : NodeLocationStore() {
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

            void callback_after_nodes() {
                if (Osmium::global.debug) {
                    std::cerr << "NodeLocationStore (Sparsetable) has size=" << nodes_table.size() << " * " << sizeof(struct coordinates) << " B (sizeof struct coordinates) = " << nodes_table.size() * sizeof(struct coordinates) / (1024 * 1204) << "MBytes" << std::endl;
                }
            }

            void callback_way(OSM::Way *object) {
                osm_sequence_id_t num_nodes = object->node_count();
                for (osm_sequence_id_t i=0; i < num_nodes; i++) {
                    struct coordinates c = nodes_table[object->nodes[i] + NodeLocationStore::negative_id_offset];
                    object->set_node_coordinates(i, fix_to_double(c.x), fix_to_double(c.y));
                }
            }

        }; // class NLS_Sparsetable

        /**
        * The NLS_Disk node location store handler stores location
        * in a memory-mapped file on disk. The size of the file is 8 times
        * the largest node ID.
        *
        * Use this node location store if the other types of storage lead
        * to memory problems.
        */
        class NLS_Disk : public NodeLocationStore {

            int max_nodes;
            struct coordinates *coordinates;

        public:

            NLS_Disk() : NodeLocationStore() {
                max_nodes = 1.2 * 1024 * 1024 * 1024; // XXX make configurable, or autosizing?
                FILE *tf = tmpfile();
                if (!tf) {
                    throw std::bad_alloc();
                }
                fseek(tf, sizeof(struct coordinates) * max_nodes-1, SEEK_SET);
                int fd = fileno(tf);
                if (fd<0) {
                    throw std::bad_alloc();
                }
                write(fd, "", 1);
                coordinates = (struct coordinates *) mmap(NULL, sizeof(struct coordinates) * max_nodes,
                              PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
                std::cout << "mmap " << max_nodes << std::endl;
                if (coordinates == MAP_FAILED) {
                    throw std::bad_alloc();
                }
            }

            ~NLS_Disk() {
                munmap(coordinates,  sizeof(struct coordinates) * max_nodes);
            }

            void callback_node(const OSM::Node *object) {
                const osm_object_id_t id = object->get_id() + NodeLocationStore::negative_id_offset;
                coordinates[id].x = double_to_fix(object->get_lon());
                coordinates[id].y = double_to_fix(object->get_lat());
            }

            void callback_after_nodes() {
                if (Osmium::global.debug) {
                    std::cerr << "NodeLocationStore (Disk) needs " << max_nodes << " * " << sizeof(struct coordinates) << "Bytes = " << max_nodes * sizeof(struct coordinates) / (1024 * 1204) << "MBytes" << std::endl;
                }
            }

            void callback_way(OSM::Way *object) {
                const osm_sequence_id_t num_nodes = object->node_count();
                for (osm_sequence_id_t i=0; i < num_nodes; i++) {
                    osm_object_id_t node_id = object->nodes[i] + NodeLocationStore::negative_id_offset;
                    object->set_node_coordinates(i, fix_to_double(coordinates[node_id].x), fix_to_double(coordinates[node_id].y));
                }
            }

        }; // class NLS_Array

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_NODELOCATIONSTORE_HPP
