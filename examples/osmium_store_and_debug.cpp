/*

  This is a small tool to dump the contents of the input file.

  If OSMIUM_DEBUG_WITH_ENDTIME is defined when compiling, the
  Osmium::Handler::EndTime is used.

*/

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/storage/objectstore.hpp>

int main(int argc, char *argv[]) {
    Osmium::init(true);

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE1 OSMFILE2\n";
        exit(1);
    }

    Osmium::OSMFile infile1(argv[1]);
    Osmium::OSMFile infile2(argv[2]);

    Osmium::Storage::ObjectStore os;
    infile1.read(os);

    Osmium::Handler::Debug debug;
    Osmium::OSM::Meta meta;
    Osmium::Storage::ObjectStore::ApplyHandler<Osmium::Handler::Debug> ah(os, &debug, meta);
    infile2.read(ah);
}

