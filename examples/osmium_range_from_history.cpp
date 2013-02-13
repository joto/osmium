/*

  Program to test the RangeFromHistory handler.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/output/xml.hpp>
#include <osmium/output/pbf.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/handler/endtime.hpp>
#include <osmium/handler/range_from_history.hpp>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " INFILE OUTFILE\n";
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    Osmium::OSMFile outfile(argv[2]);

    Osmium::Output::Handler out(outfile);
    Osmium::Handler::RangeFromHistory<Osmium::Output::Handler> range_handler(out, time(0), time(0));
    Osmium::Handler::EndTime<Osmium::Handler::RangeFromHistory<Osmium::Output::Handler> > handler(range_handler);
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

