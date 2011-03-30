
#include <osmium.hpp>
#include "HandlerNodeDensity.hpp"

Osmium::Handler::NodeDensity *osmium_handler_node_density;

void node_handler(Osmium::OSM::Node *node) {
    osmium_handler_node_density->callback_node(node);
}

void after_nodes_handler() {
    osmium_handler_node_density->callback_after_nodes();
    exit(0); // XXX this is suboptimal as objects don't get destructed properly
}

struct callbacks *setup_callbacks() {
    static struct callbacks cb;
    cb.node        = node_handler;
    cb.after_nodes = after_nodes_handler;
    return &cb;
}

bool debug;

/* ================================================== */

int main(int argc, char *argv[]) {
    debug = false;

    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE SIZE MIN MAX" << std::endl;
        exit(1);
    }

    int size = atoi(argv[2]);
    int min  = atoi(argv[3]);
    int max  = atoi(argv[4]);

    osmium_handler_node_density = new Osmium::Handler::NodeDensity(size, min, max);

    parse_osmfile(argv[1], setup_callbacks());

    delete osmium_handler_node_density;

    // this is needed even if the protobuf lib was never used so that valgrind doesn't report any errors
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}

