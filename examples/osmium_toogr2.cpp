/*

  This is an example tool that converts OSM data to a spatialite database using
  the OGR library.

  This version creates multipolygons and reads the input file twice to do that.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#include <ogr_api.h>
#include <ogrsf_frmts.h>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/storage/byid/sparse_table.hpp>
#include <osmium/storage/byid/mmap_file.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/multipolygon/assembler.hpp>
#include <osmium/geometry/multipolygon.hpp>
#include <osmium/geometry/ogr.hpp>
#include <osmium/geometry/ogr_multipolygon.hpp>

/* ================================================== */

class OGROutHandler : public Osmium::Handler::Base {

    OGRDataSource* m_data_source;
    OGRLayer* m_layer_mp;

public:

    OGROutHandler() : Osmium::Handler::Base() {
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

    ~OGROutHandler() {
        OGRDataSource::DestroyDataSource(m_data_source);
        OGRCleanupAll();
    }

    void area(const shared_ptr<Osmium::OSM::Area const>& area) {
        const char* building = area->tags().get_value_by_key("building");
        if (building) {
            try {
                Osmium::Geometry::MultiPolygon mp(*area);

                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_mp->GetLayerDefn());
                OGRMultiPolygon* ogrmp = Osmium::Geometry::create_ogr_geometry(mp);
                feature->SetGeometry(ogrmp);
                // there are not so many multipolygon relations, so these ids will still fit into a 32bit int
                feature->SetField("id", static_cast<int>(area->id()));

                if (m_layer_mp->CreateFeature(feature) != OGRERR_NONE) {
                    std::cerr << "Failed to create feature.\n";
                    exit(1);
                }

                OGRFeature::DestroyFeature(feature);
                delete ogrmp;
            } catch (Osmium::Geometry::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for multipolygon " << area->id() << ".\n";
            }
        }
    }

};

/* ================================================== */

typedef Osmium::Storage::ById::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);

    bool attempt_repair = true;

    storage_sparsetable_t store_pos;
    storage_mmap_t store_neg;

    OGROutHandler ogr_out_handler;

    typedef Osmium::MultiPolygon::Assembler<OGROutHandler> assembler_t;
    assembler_t assembler(ogr_out_handler, attempt_repair);
    assembler.set_debug_level(1);

    typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;
    cfw_handler_t cfw_handler(store_pos, store_neg);

    typedef Osmium::Handler::Sequence<cfw_handler_t, assembler_t::HandlerPass2> sequence_handler_t;
    sequence_handler_t sequence_handler(cfw_handler, assembler.handler_pass2());

    std::cerr << "First pass...\n";
    Osmium::Input::read(infile, assembler.handler_pass1());
    std::cout << "Used memory: " << assembler.used_memory() / (1024 * 1024) << " MB" << std::endl;

    std::cerr << "Second pass...\n";
    Osmium::Input::read(infile, sequence_handler);

    google::protobuf::ShutdownProtobufLibrary();
}

