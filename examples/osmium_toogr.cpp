/*

  This is an example tool that converts OSM data to some output format
  like Spatialite or Shapefiles using the OGR library.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>
#include <getopt.h>

#include <ogr_api.h>
#include <ogrsf_frmts.h>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

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

    MyOGRHandler(const std::string& driver_name, const std::string& filename) {
        handler_cfw = new cfw_handler_t(store_pos, store_neg);

        OGRRegisterAll();

        OGRSFDriver* driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(driver_name.c_str());
        if (driver == NULL) {
            std::cerr << driver_name << " driver not available.\n";
            exit(1);
        }

        CPLSetConfigOption("OGR_SQLITE_SYNCHRONOUS", "FALSE");
        const char* options[] = { "SPATIALITE=TRUE", NULL };
        m_data_source = driver->CreateDataSource(filename.c_str(), const_cast<char**>(options));
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

        OGRFieldDefn layer_point_field_id("id", OFTReal);
        layer_point_field_id.SetWidth(10);

        if (m_layer_point->CreateField(&layer_point_field_id) != OGRERR_NONE) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_point_field_operator("operator", OFTString);
        layer_point_field_operator.SetWidth(30);

        if (m_layer_point->CreateField(&layer_point_field_operator) != OGRERR_NONE) {
            std::cerr << "Creating operator field failed.\n";
            exit(1);
        }

        /* Transactions might make things faster, then again they might not.
           Feel free to experiment and benchmark and report back. */
//        m_layer_point->StartTransaction();

        m_layer_linestring = m_data_source->CreateLayer("roads", &sparef, wkbLineString, NULL);
        if (m_layer_linestring == NULL) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_linestring_field_id("id", OFTReal);
        layer_linestring_field_id.SetWidth(10);

        if (m_layer_linestring->CreateField(&layer_linestring_field_id) != OGRERR_NONE) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_linestring_field_type("type", OFTString);
        layer_linestring_field_type.SetWidth(30);

        if (m_layer_linestring->CreateField(&layer_linestring_field_type) != OGRERR_NONE) {
            std::cerr << "Creating type field failed.\n";
            exit(1);
        }

//        m_layer_linestring->StartTransaction();
    }

    ~MyOGRHandler() {
        OGRDataSource::DestroyDataSource(m_data_source);
        delete handler_cfw;
        OGRCleanupAll();
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

                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_point->GetLayerDefn());
                OGRPoint* ogrpoint = Osmium::Geometry::create_ogr_geometry(point);
                feature->SetGeometry(ogrpoint);
                feature->SetField("id", static_cast<double>(node->id()));
                feature->SetField("operator", node->tags().get_value_by_key("operator"));

                if (m_layer_point->CreateFeature(feature) != OGRERR_NONE) {
                    std::cerr << "Failed to create feature.\n";
                    exit(1);
                }

                OGRFeature::DestroyFeature(feature);
                delete ogrpoint;
            } catch (Osmium::Geometry::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for node " << node->id() << ".\n";
            }
        }
    }

    void after_nodes() {
//        m_layer_point->CommitTransaction();
        std::cerr << "Memory used for node coordinates storage (approximate):\n  for positive IDs: "
                  << store_pos.used_memory() / (1024 * 1024)
                  << " MiB\n  for negative IDs: "
                  << store_neg.used_memory() / (1024 * 1024)
                  << " MiB\n";
        handler_cfw->after_nodes();
    }

    void way(const shared_ptr<Osmium::OSM::Way>& way) {
        handler_cfw->way(way);
        const char* highway = way->tags().get_value_by_key("highway");
        if (highway) {
            try {
                Osmium::Geometry::LineString linestring(*way);

                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_linestring->GetLayerDefn());
                OGRLineString* ogrlinestring = Osmium::Geometry::create_ogr_geometry(linestring);
                feature->SetGeometry(ogrlinestring);
                feature->SetField("id", static_cast<double>(way->id()));
                feature->SetField("type", highway);

                if (m_layer_linestring->CreateFeature(feature) != OGRERR_NONE) {
                    std::cerr << "Failed to create feature.\n";
                    exit(1);
                }

                OGRFeature::DestroyFeature(feature);
                delete ogrlinestring;
            } catch (Osmium::Geometry::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for way " << way->id() << ".\n";
            }
        }
    }

//    void after_ways() {
//        m_layer_linestring->CommitTransaction();
//    }

};

/* ================================================== */

void print_help() {
    std::cout << "osmium_toogr [OPTIONS] [INFILE [OUTFILE]]\n\n" \
              << "If INFILE is not given stdin is assumed.\n" \
              << "If OUTFILE is not given 'ogr_out' is used.\n" \
              << "\nOptions:\n" \
              << "  -h, --help           This help message\n" \
              << "  -f, --format=FORMAT  Output OGR format (Default: 'SQLite')\n";
}

int main(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"help",   no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    std::string output_format("SQLite");

    while (true) {
        int c = getopt_long(argc, argv, "hf:", long_options, 0);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'h':
                print_help();
                exit(0);
            case 'f':
                output_format = optarg;
                break;
            default:
                exit(1);
        }
    }

    std::string input_filename;
    std::string output_filename("ogr_out");
    int remaining_args = argc - optind;
    if (remaining_args > 2) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] [INFILE [OUTFILE]]" << std::endl;
        exit(1);
    } else if (remaining_args == 2) {
        input_filename =  argv[optind];
        output_filename = argv[optind+1];
    } else if (remaining_args == 1) {
        input_filename =  argv[optind];
    } else {
        input_filename = "-";
    }

    Osmium::OSMFile infile(input_filename);
    MyOGRHandler handler(output_format, output_filename);
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

