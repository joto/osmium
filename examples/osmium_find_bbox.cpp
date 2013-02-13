/*

  This is a small tool to find the bounding box of the input file.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/handler/find_bbox.hpp>

/* ================================================== */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    Osmium::Handler::FindBbox handler;
    Osmium::Input::read(infile, handler);

    Osmium::OSM::Bounds bounds = handler.bounds();
    std::cout <<  "minlon=" << bounds.bottom_left().lon()
              << " minlat=" << bounds.bottom_left().lat()
              << " maxlon=" << bounds.top_right().lon()
              << " maxlat=" << bounds.top_right().lat() << std::endl;

    google::protobuf::ShutdownProtobufLibrary();
}

