/*

  This is an example tool that converts OSM data to a spatialite database using
  the OGR library.

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

#include <cstdlib>

#include <ogrsf_frmts.h>

#include <osmium.hpp>
#include <osmium/storage/byid/sparse_table.hpp>
#include <osmium/storage/byid/mmap_file.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/geometry/ogr.hpp>

typedef Osmium::Storage::ById::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;
typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;

class MyOGRHandler : public Osmium::Handler::Base {

    OGRDataSource* m_data_source;
    OGRLayer* m_layer_point;
    OGRLayer* m_layer_linestring;

    storage_sparsetable_t store_pos;
    storage_mmap_t store_neg;
    cfw_handler_t* handler_cfw;

public:

    MyOGRHandler() {
        handler_cfw = new cfw_handler_t(store_pos, store_neg);

        OGRRegisterAll();

        const char* driver_name = "SQLite";
        OGRSFDriver* driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(driver_name);
        if (driver == NULL) {
            std::cerr << driver_name << " driver not available.\n";
            exit(1);
        }

        CPLSetConfigOption("OGR_SQLITE_SYNCHRONOUS", "FALSE");
        const char* options[] = { "SPATIALITE=TRUE", NULL };
        m_data_source = driver->CreateDataSource("ogr_out.sqlite", const_cast<char**>(options));
        if (m_data_source == NULL) {
            std::cerr << "Creation of output file failed.\n";
            exit(1);
        }

        OGRSpatialReference sparef;
        sparef.SetWellKnownGeogCS("WGS84");
        m_layer_point = m_data_source->CreateLayer("postboxes", &sparef, wkbPoint, NULL);
        if (m_layer_point == NULL) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_point_field_id("id", OFTInteger);
        layer_point_field_id.SetWidth(10);

        if (m_layer_point->CreateField(&layer_point_field_id) != OGRERR_NONE ) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_point_field_operator("operator", OFTString);
        layer_point_field_operator.SetWidth(30);

        if (m_layer_point->CreateField(&layer_point_field_operator) != OGRERR_NONE ) {
            std::cerr << "Creating operator field failed.\n";
            exit(1);
        }

        m_layer_linestring = m_data_source->CreateLayer("roads", &sparef, wkbLineString, NULL);
        if (m_layer_linestring == NULL) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_linestring_field_id("id", OFTInteger);
        layer_linestring_field_id.SetWidth(10);

        if (m_layer_linestring->CreateField(&layer_linestring_field_id) != OGRERR_NONE ) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_linestring_field_type("type", OFTString);
        layer_linestring_field_type.SetWidth(30);

        if (m_layer_linestring->CreateField(&layer_linestring_field_type) != OGRERR_NONE ) {
            std::cerr << "Creating type field failed.\n";
            exit(1);
        }
    }

    ~MyOGRHandler() {
        OGRDataSource::DestroyDataSource(m_data_source);
        delete handler_cfw;
    }

    void init(Osmium::OSM::Meta& meta) {
        handler_cfw->init(meta);
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& node) {
        handler_cfw->node(node);
        const char* amenity = node->tags().get_tag_by_key("amenity");
        if (amenity && !strcmp(amenity, "post_box")) {
            try {
                Osmium::Geometry::Point point(*node);

                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_point->GetLayerDefn());
                OGRPoint* ogrpoint = Osmium::Geometry::create_ogr_geometry(point);
                feature->SetGeometry(ogrpoint);
                feature->SetField("id", node->id());
                feature->SetField("operator", node->tags().get_tag_by_key("operator"));

                if (m_layer_point->CreateFeature(feature) != OGRERR_NONE) {
                    std::cerr << "Failed to create feature.\n";
                    exit(1);
                }

                OGRFeature::DestroyFeature(feature);
                delete ogrpoint;
            } catch (Osmium::Exception::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for node " << node->id() << ".\n";
            }
        }
    }

    void after_nodes() {
        std::cerr << "Memory used for node coordinates storage (approximate):\n  for positive IDs: "
                  << store_pos.used_memory() / (1024 * 1024)
                  << " MiB\n  for negative IDs: "
                  << store_neg.used_memory() / (1024 * 1024)
                  << " MiB\n";
        handler_cfw->after_nodes();
    }

    void way(const shared_ptr<Osmium::OSM::Way>& way) {
        handler_cfw->way(way);
        const char* highway = way->tags().get_tag_by_key("highway");
        if (highway) {
            try {
                Osmium::Geometry::LineString linestring(*way);

                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_linestring->GetLayerDefn());
                OGRLineString* ogrlinestring = Osmium::Geometry::create_ogr_geometry(linestring);
                feature->SetGeometry(ogrlinestring);
                feature->SetField("id", way->id());
                feature->SetField("type", highway);

                if (m_layer_linestring->CreateFeature(feature) != OGRERR_NONE) {
                    std::cerr << "Failed to create feature.\n";
                    exit(1);
                }

                OGRFeature::DestroyFeature(feature);
                delete ogrlinestring;
            } catch (Osmium::Exception::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for way " << way->id() << ".\n";
            }
        }
    }
};

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::init(true);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    MyOGRHandler handler;
    infile.read(handler);
}

