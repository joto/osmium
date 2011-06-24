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
#include <osmium/utils/coordinates.hpp>
#include <osmium/storage/byid.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/output/shapefile.hpp>

typedef Osmium::Storage::SparseTable<Osmium::Coordinates> storage_sparsetable_t;
typedef Osmium::Storage::Mmap<Osmium::Coordinates> storage_mmap_t;
typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;

class MyShapeHandler : public Osmium::Handler::Base {

    Osmium::Output::PointShapefile *shapefile_point;
    Osmium::Output::LineStringShapefile *shapefile_linestring;

    storage_sparsetable_t store_pos;
    storage_mmap_t store_neg;
    cfw_handler_t* handler_cfw;

public:

    MyShapeHandler() {
        handler_cfw = new cfw_handler_t(store_pos, store_neg);
        std::string filename("postboxes");
        shapefile_point = new Osmium::Output::PointShapefile(filename);
        shapefile_point->add_field("id", FTInteger, 10);
        shapefile_point->add_field("operator", FTString, 30);
        filename = "roads";
        shapefile_linestring = new Osmium::Output::LineStringShapefile(filename);
        shapefile_linestring->add_field("id", FTInteger, 10);
        shapefile_linestring->add_field("type", FTString, 30);
    }

    ~MyShapeHandler() {
        delete shapefile_linestring;
        delete shapefile_point;
    }

    void callback_init() {
        handler_cfw->callback_init();
    }

    void callback_node(Osmium::OSM::Node *node) {
        handler_cfw->callback_node(node);
        const char *amenity = node->get_tag_by_key("amenity");
        if (amenity && !strcmp(amenity, "post_box")) {
            try {
                Osmium::Geometry::Point point(*node);
                shapefile_point->add_geometry(point.create_shp_object());
                shapefile_point->add_attribute(0, node->get_id());
                const char *op = node->get_tag_by_key("operator");
                if (op) {
                    shapefile_point->add_attribute(1, std::string(op));
                }
            } catch (Osmium::Exception::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for node " << node->get_id() << ".\n";
            }
        }
    }

    void callback_after_nodes() {
        handler_cfw->callback_after_nodes();
    }

    void callback_way(Osmium::OSM::Way *way) {
        handler_cfw->callback_way(way);
        const char *highway = way->get_tag_by_key("highway");
        if (highway) {
            try {
                Osmium::Geometry::LineString linestring(*way);
                shapefile_linestring->add_geometry(linestring.create_shp_object());
                shapefile_linestring->add_attribute(0, way->get_id());
                const char *type = way->get_tag_by_key("highway");
                if (type) {
                    shapefile_linestring->add_attribute(1, std::string(type));
                }
            } catch (Osmium::Exception::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for way " << way->get_id() << ".\n";
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

