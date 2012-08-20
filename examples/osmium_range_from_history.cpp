/*

  Program to test the RangeFromHistory handler.

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
#include <osmium/output/xml.hpp>
#include <osmium/output/pbf.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/handler/endtime.hpp>
#include <osmium/handler/range_from_history.hpp>

int main(int argc, char *argv[]) {
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

