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
#include "handler_nodedensity.hpp"

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::init();

    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE SIZE MIN MAX\n\n";
        std::cerr << "  OSMFILE - OSM file of any type.\n";
        std::cerr << "  SIZE    - Y-size of resulting image (X-size will be double).\n";
        std::cerr << "  MIN     - Node counts smaller than this will be black.\n";
        std::cerr << "  MAX     - Node counts larger than this will be white.\n\n";
        std::cerr << "Output will be a PNG file on STDOUT. Make sure to redirect.\n";
        exit(1);
    }

    int size = atoi(argv[2]);
    int min  = atoi(argv[3]);
    int max  = atoi(argv[4]);

    Osmium::OSMFile infile(argv[1]);
    Osmium::Handler::NodeDensity handler(size, min, max);
    infile.read(handler);
}

