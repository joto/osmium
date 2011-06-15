/*

  This is a small tool to find the bounding box of the input file.

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

#include <cstdlib>

#define OSMIUM_MAIN
#include <osmium.hpp>
#include <osmium/handler/find_bbox.hpp>

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::Framework osmium;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    Osmium::Handler::FindBbox handler;
    infile.read(handler);

    std::cout <<  "minlon=" << handler.get_minlon()
              << " minlat=" << handler.get_minlat()
              << " maxlon=" << handler.get_maxlon()
              << " maxlat=" << handler.get_maxlat() << std::endl;
}

