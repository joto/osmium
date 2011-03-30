
#include <osmium.hpp>
#include "HandlerNodeDensity.hpp"

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

    Osmium::Handler::NodeDensity *handler_node_density = new Osmium::Handler::NodeDensity(size, min, max);
    parse_osmfile<Osmium::Handler::NodeDensity>(argv[1], handler_node_density);

    // this is needed even if the protobuf lib was never used so that valgrind doesn't report any errors
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}

