/*

  This is a small tool to try the Statistics handler

*/

#include <cstdlib>

#define OSMIUM_MAIN
#include <osmium.hpp>
#include <osmium/handler/statistics.hpp>

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::Framework osmium;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    osmium.parse_osmfile<Osmium::Handler::Statistics>(argv[1]);
}

