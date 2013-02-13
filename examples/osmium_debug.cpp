/*

  This is a small tool to dump the contents of the input file.

  If OSMIUM_DEBUG_WITH_ENDTIME is defined when compiling, the
  Osmium::Handler::EndTime is used.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>

#ifdef OSMIUM_DEBUG_WITH_ENDTIME
# include <osmium/handler/endtime.hpp>
#endif // OSMIUM_DEBUG_WITH_ENDTIME

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
#ifdef OSMIUM_DEBUG_WITH_ENDTIME
    Osmium::Handler::Debug debug_handler(true);
    Osmium::Handler::EndTime<Osmium::Handler::Debug> handler(debug_handler);
#else
    Osmium::Handler::Debug handler;
#endif // OSMIUM_DEBUG_WITH_ENDTIME
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

