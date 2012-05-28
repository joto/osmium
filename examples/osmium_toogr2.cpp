/*

  This is an example tool that converts OSM data to a spatialite database using
  the OGR library.

  This version creates multipolygons and reads the input file twice to do that.

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
#include <osmium/storage/byid/sparsetable.hpp>
#include <osmium/storage/byid/mmap_file.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/handler/multipolygon.hpp>
#include <osmium/geometry/multipolygon.hpp>

typedef Osmium::Storage::ById::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;
typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;

class MyOGRHandler : public Osmium::Handler::BaseWithArea {

    OGRDataSource* m_data_source;
    OGRLayer* m_layer_mp;

public:

    MyOGRHandler(bool attempt_repair) : Osmium::Handler::BaseWithArea(attempt_repair) {

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
        m_layer_mp = m_data_source->CreateLayer("areas", &sparef, wkbMultiPolygon, NULL);
        if (m_layer_mp == NULL) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_mp_field_id("id", OFTInteger);
        layer_mp_field_id.SetWidth(10);

        if (m_layer_mp->CreateField(&layer_mp_field_id) != OGRERR_NONE) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }
    }

    ~MyOGRHandler() {
        OGRDataSource::DestroyDataSource(m_data_source);
    }

    // TODO: filter interesting relations for Multipolygon-Processing

    void _node(const shared_ptr<Osmium::OSM::Node>& node) const {
        std::cerr << "Node #" << node->id() << std::endl;
    }

    void _way(const shared_ptr<Osmium::OSM::Way>& way) const {
        std::cerr << "Way #" << way->id() << std::endl;
    }

    void _relation(const shared_ptr<Osmium::OSM::Relation>& relation) const {
        std::cerr << "Relation #" << relation->id() << std::endl;
    }

    void _area(Osmium::OSM::Area* area) {
        std::cerr << "Area from " << ((area->get_type() == AREA_FROM_WAY) ? "Way" : "Relation")
            << " #" << area->id() << std::endl;

        const char* building = area->tags().get_tag_by_key("building");
        if (building) {
            try {
                Osmium::Geometry::MultiPolygon mp(*area);

                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_mp->GetLayerDefn());
                OGRMultiPolygon* ogrmp = mp.create_ogr_geometry();

                feature->SetGeometry(ogrmp);
                feature->SetField("id", area->id());

                if (m_layer_mp->CreateFeature(feature) != OGRERR_NONE) {
                    std::cerr << "Failed to create feature.\n";
                    exit(1);
                }

                OGRFeature::DestroyFeature(feature);
                delete ogrmp;
            } catch (Osmium::Exception::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for multipolygon " << area->id() << ".\n";
            }
        }
    }
};

int main(int argc, char *argv[]) {
    Osmium::init(true);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);

    bool attempt_repair = true;

    std::cerr << "Initializing" << std::endl;
    MyOGRHandler handler(attempt_repair);

    std::cerr << "Starting 1st Pass" << std::endl;
    infile.read(handler);

    std::cerr << "Starting 2nd Pass" << std::endl;
    infile.read(handler);

    std::cerr << "Done" << std::endl;
}
