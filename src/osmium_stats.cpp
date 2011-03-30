/*

  This is a small tool to try the Statistics handler

*/

#include <stdlib.h>
#include "osmium.hpp"
#include <HandlerStatistics.hpp>

bool debug;

/* ================================================== */

int main(int argc, char *argv[]) {
    debug = true;
    Osmium::Framework osmium;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    osmium.parse_osmfile<Osmium::Handler::Statistics>(argv[1]);
}

