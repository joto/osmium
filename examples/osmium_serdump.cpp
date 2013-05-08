/*

  This is a small tool to dump the contents of the input file
  in serialized format to stdout.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/ser/handler.hpp>

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    Osmium::Ser::BufferManager::Malloc manager(10000);
    Osmium::Ser::Handler<Osmium::Ser::BufferManager::Malloc> handler(manager);
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

