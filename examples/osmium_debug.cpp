/*

  This is a small tool to dump the contents of the input file.

  If OSMIUM_DEBUG_WITH_ENDTIME is defined when compiling, the
  Osmium::Handler::EndTime is used.

*/

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

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

