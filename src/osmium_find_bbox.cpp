/*

  This is a small tool to find the bounding box of the input file.

*/

#include <cstdlib>

#define OSMIUM_MAIN
#include <osmium.hpp>
#include <osmium/handler/find_bbox.hpp>

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::Framework osmium;
    Osmium::Handler::FindBbox handler;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    osmium.parse_osmfile<Osmium::Handler::FindBbox>(argv[1], &handler);

    std::cout <<  "minlon=" << handler.get_minlon()
              << " maxlon=" << handler.get_maxlon()
              << " minlat=" << handler.get_minlat()
              << " maxlat=" << handler.get_maxlat() << std::endl;
}

