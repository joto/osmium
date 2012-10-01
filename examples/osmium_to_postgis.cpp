/*

  This is an example tool that loads OSM data into a PostGIS
  database with hstore tags column using the OGR library.

  The database must have the HSTORE and POSTGIS extentions
  loaded.

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

#include <getopt.h>
#include <iostream>
#include <numeric>
#include <string>

#include <ogr_api.h>
#include <ogrsf_frmts.h>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/utils/filter_and_accumulate.hpp>
#include <osmium/tags/key_filter.hpp>
#include <osmium/tags/to_string.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/geometry/ogr.hpp>

class MyOGRHandler : public Osmium::Handler::Base {

    OGRDataSource* m_data_source;
    OGRLayer* m_layer_point;
    Osmium::Tags::KeyFilter m_filter;
    Osmium::Tags::TagToHStoreStringOp m_tohstore;

public:

    MyOGRHandler(const std::string& filename) :
        m_data_source(NULL),
        m_layer_point(NULL),
        m_filter(true),
        m_tohstore() {
        OGRRegisterAll();

        OGRSFDriver* driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("PostgreSQL");
        if (driver == NULL) {
            std::cerr << "PostgreSQL OGR driver not available.\n";
            exit(1);
        }

        // using COPY is much faster than INSERT
        CPLSetConfigOption("PG_USE_COPY", "YES");
        const char* options[] = { NULL };
        m_data_source = driver->CreateDataSource(filename.c_str(), const_cast<char**>(options));
        if (m_data_source == NULL) {
            std::cerr << "Database open failed.\n";
            exit(1);
        }

        // OGR can't create a table with hstore column, so we do it ourselves here
        m_data_source->ExecuteSQL("CREATE TABLE nodes (id BIGINT, tags hstore);", NULL, NULL);
        m_data_source->ExecuteSQL("SELECT AddGeometryColumn('nodes', 'geom', 4326, 'POINT', 2);", NULL, NULL);

        m_layer_point = m_data_source->GetLayerByName("nodes");

        // using transactions make this much faster than without
        m_layer_point->StartTransaction();

        m_filter.add(false, "created_by");
        m_filter.add(false, "odbl");
    }

    ~MyOGRHandler() {
        OGRDataSource::DestroyDataSource(m_data_source);
        OGRCleanupAll();
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& node) {
        if (!node->tags().empty()) {
            std::string tags;

            Osmium::filter_and_accumulate(node->tags(), m_filter, tags, m_tohstore);

            if (!tags.empty()) {
                try {
                    Osmium::Geometry::Point point(*node);

                    OGRFeature* feature = OGRFeature::CreateFeature(m_layer_point->GetLayerDefn());
                    OGRPoint* ogrpoint = Osmium::Geometry::create_ogr_geometry(point);
                    feature->SetGeometry(ogrpoint);
                    feature->SetField("id", node->id());
                    feature->SetField("tags", tags.c_str());

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
    }

};

/* ================================================== */

void print_help() {
    std::cout << "osmium_to_postgis [OPTIONS] INFILE DATABASE\n\n" \
              << "\nOptions:\n" \
              << "  -h, --help           This help message\n" \
              << "  -d, --debug          Enable debugging output\n";
}

int main(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"debug",  no_argument, 0, 'd'},
        {"help",   no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    bool debug = false;

    while (true) {
        int c = getopt_long(argc, argv, "dh", long_options, 0);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'd':
                debug = true;
                break;
            case 'h':
                print_help();
                exit(0);
            default:
                exit(1);
        }
    }

    std::string input_filename;
    std::string database_descriptor("PG:dbname=");
    int remaining_args = argc - optind;
    if (remaining_args != 2) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] INFILE DATABASE" << std::endl;
        exit(1);
    } else {
        input_filename = argv[optind];
        database_descriptor += argv[optind+1];
    }

    if (debug) {
        std::cout << "Reading file '" << input_filename << "'\nOpening database '" << database_descriptor << "'\n";
    }

    Osmium::OSMFile infile(input_filename);
    MyOGRHandler handler(database_descriptor);
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

