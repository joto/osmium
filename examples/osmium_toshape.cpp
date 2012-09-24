/*

  This is an example tool that converts OSM data to a shapefile.

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
#include <osmium/storage/byid/sparse_table.hpp>
#include <osmium/storage/byid/mmap_file.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/export/shapefile.hpp>

typedef Osmium::Storage::ById::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;
typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;

class MyShapeHandler : public Osmium::Handler::Base {

    Osmium::Export::PointShapefile* shapefile_point;
    Osmium::Export::LineStringShapefile* shapefile_linestring;

    storage_sparsetable_t store_pos;
    storage_mmap_t store_neg;
    cfw_handler_t* handler_cfw;

public:

    MyShapeHandler() {
        handler_cfw = new cfw_handler_t(store_pos, store_neg);
        shapefile_point = new Osmium::Export::PointShapefile("postboxes");
        shapefile_point->add_field("id", FTInteger, 10);
        shapefile_point->add_field("operator", FTString, 30);
        shapefile_linestring = new Osmium::Export::LineStringShapefile("roads");
        shapefile_linestring->add_field("id", FTInteger, 10);
        shapefile_linestring->add_field("type", FTString, 30);
    }

    ~MyShapeHandler() {
        delete shapefile_linestring;
        delete shapefile_point;
    }

    void init(Osmium::OSM::Meta& meta) {
        handler_cfw->init(meta);
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& node) {
        handler_cfw->node(node);
        const char* amenity = node->tags().get_value_by_key("amenity");
        if (amenity && !strcmp(amenity, "post_box")) {
            try {
                Osmium::Geometry::Point point(*node);
                shapefile_point->add_geometry(Osmium::Geometry::create_shp_object(point));
                shapefile_point->add_attribute(0, node->id());
                const char* op = node->tags().get_value_by_key("operator");
                if (op) {
                    shapefile_point->add_attribute_with_truncate(1, std::string(op));
                }
            } catch (Osmium::Geometry::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for node " << node->id() << ".\n";
            }
        }
    }

    void after_nodes() {
        handler_cfw->after_nodes();
    }

    void way(const shared_ptr<Osmium::OSM::Way>& way) {
        handler_cfw->way(way);
        const char* highway = way->tags().get_value_by_key("highway");
        if (highway) {
            try {
                Osmium::Geometry::LineString linestring(*way);
                shapefile_linestring->add_geometry(Osmium::Geometry::create_shp_object(linestring));
                shapefile_linestring->add_attribute(0, way->id());
                shapefile_linestring->add_attribute_with_truncate(1, std::string(highway));
            } catch (Osmium::Geometry::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for way " << way->id() << ".\n";
            }
        }
    }
};

/* ================================================== */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    MyShapeHandler handler;
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

