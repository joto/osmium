/*

  Reads a .poly file and prints its content as WKT

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
#include <osmium/utils/geometryreader.hpp>

#include <geos/geom/Geometry.h>
#include <geos/io/WKTWriter.h>

int main(int argc, char *argv[]) {
    Osmium::init(true);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " INFILE" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string suffix(filename.substr(filename.find_first_of('.')+1));
    geos::geom::Geometry *geom = NULL;

    if (suffix == "poly") {
        geom = Osmium::GeometryReader::fromPolyFile(filename);
    } else if (suffix == "osm") {
        geom = Osmium::GeometryReader::fromOsmFile(filename);
    } else {
        std::cerr << "Unknown suffix " << suffix << std::endl;
    }
    //geos::geom::Geometry *geom = Osmium::GeometryReader::fromBBox(-180, -90, 180, 90);
    //geos::geom::Geometry *geom = Osmium::GeometryReader::fromBBox("-180,-90,180,90");

    if (!geom) {
        std::cerr << "Unable to read polygon file: " << filename << std::endl;
        return 1;
    }

    geos::io::WKTWriter *w = new geos::io::WKTWriter();
    std::string wkt = w->write(geom);
    std::cout << wkt << std::endl;

    Osmium::Geometry::geos_geometry_factory()->destroyGeometry(geom);
    delete w;

    return 0;
}

