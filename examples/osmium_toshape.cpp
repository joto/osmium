/*

  This is an example tool that converts OSM data to a shapefile.

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
#include <osmium/output/shapefile.hpp>

class MyShapeHandler : public Osmium::Handler::Base {

    Osmium::Output::PointShapefile *shapefile;

public:

    MyShapeHandler() {
        std::string filename("postboxes");
        shapefile = new Osmium::Output::PointShapefile(filename);
        shapefile->add_field("id", FTInteger, 10);
        shapefile->add_field("operator", FTString, 30);
    }

    ~MyShapeHandler() {
        delete shapefile;
    }

    void callback_node(Osmium::OSM::Node *node) {
        const char *amenity = node->get_tag_by_key("amenity");
        if (amenity && !strcmp(amenity, "post_box")) {
            try {
                shapefile->add_geometry(shapefile->get_geometry(node));
                shapefile->add_attribute(0, node->get_id());
                const char *op = node->get_tag_by_key("operator");
                if (op) {
                    shapefile->add_attribute(1, std::string(op));
                }
            } catch (Osmium::Exception::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for node " << node->get_id() << ".\n";
            }
        }
    }
};

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::Framework osmium(true);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    MyShapeHandler handler;
    infile.read(handler);
}

