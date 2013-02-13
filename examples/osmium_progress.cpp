/*

  This is a small tool demonstrates the use of the progress handler.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/handler/progress.hpp>

/* ================================================== */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    Osmium::Handler::Progress handler;
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

