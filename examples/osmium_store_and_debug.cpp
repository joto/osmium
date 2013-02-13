/*

  This example program shows how to read an OSM change file and
  apply it to an OSM file. The results are dumped to stdout.

  The code in this example file is released into the Public Domain.

*/

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/storage/objectstore.hpp>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSM-CHANGE-FILE OSM-FILE\n";
        exit(1);
    }

    Osmium::OSMFile infile1(argv[1]);
    Osmium::OSMFile infile2(argv[2]);

    Osmium::Storage::ObjectStore os;
    Osmium::Input::read(infile1, os);

    Osmium::Handler::Debug debug;
    Osmium::OSM::Meta meta;
    Osmium::Storage::ObjectStore::ApplyHandler<Osmium::Handler::Debug> ah(os, debug, meta);
    Osmium::Input::read(infile2, ah);

    google::protobuf::ShutdownProtobufLibrary();
}

