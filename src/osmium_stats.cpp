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

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    parse_osmfile<Osmium::Handler::Statistics>(argv[1]);

    // this is needed even if the protobuf lib was never used so that valgrind doesn't report any errors
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}

