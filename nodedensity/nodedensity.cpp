
#include <osmium.hpp>
#include "handler_nodedensity.hpp"

bool debug;

/* ================================================== */

int main(int argc, char *argv[]) {
    debug = false;
    Osmium::Framework osmium;

    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE SIZE MIN MAX" << std::endl;
        exit(1);
    }

    int size = atoi(argv[2]);
    int min  = atoi(argv[3]);
    int max  = atoi(argv[4]);

    Osmium::Handler::NodeDensity *handler_node_density = new Osmium::Handler::NodeDensity(size, min, max);
    osmium.parse_osmfile<Osmium::Handler::NodeDensity>(argv[1], handler_node_density);
}

