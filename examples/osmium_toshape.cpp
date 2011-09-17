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

#include <unistd.h>
#include <getopt.h>

#include <osmium.hpp>
#include <osmium/storage/byid.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/export/shapefile.hpp>
#include <shapefil.h>

typedef Osmium::Storage::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
typedef Osmium::Storage::Mmap<Osmium::OSM::Position> storage_mmap_t;
typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;

class MyShapeHandler : public Osmium::Handler::Base {

    Osmium::Export::PolygonShapefile *shapefile_buildings;
    Osmium::Export::LineStringShapefile *shapefile_waterways;
    Osmium::Export::LineStringShapefile *shapefile_roads;
    Osmium::Export::LineStringShapefile *shapefile_railways;
    Osmium::Export::PointShapefile *shapefile_points;
    Osmium::Export::PointShapefile *shapefile_places;
    Osmium::Export::PolygonShapefile *shapefile_natural;
    Osmium::Export::PolygonShapefile *shapefile_landuse;

    storage_sparsetable_t store_pos;
    storage_mmap_t store_neg;
    cfw_handler_t* handler_cfw;

public:

    MyShapeHandler() {
        handler_cfw = new cfw_handler_t(store_pos, store_neg);

        std::string fn("buildings");
        shapefile_buildings = new Osmium::Export::PolygonShapefile(fn);
        shapefile_buildings->add_field("osm_id", FTInteger, 11);
        shapefile_buildings->add_field("name", FTString, 48);
        shapefile_buildings->add_field("type", FTString, 16);

        fn.assign("waterways");
        shapefile_waterways = new Osmium::Export::LineStringShapefile(fn);
        shapefile_waterways->add_field("osm_id", FTInteger, 11);
        shapefile_waterways->add_field("name", FTString, 48);
        shapefile_waterways->add_field("type", FTString, 16);
        shapefile_waterways->add_field("width", FTInteger, 3);

        fn.assign("roads");
        shapefile_roads = new Osmium::Export::LineStringShapefile(fn);
        shapefile_roads->add_field("osm_id", FTInteger, 11);
        shapefile_roads->add_field("name", FTString, 48);
        shapefile_roads->add_field("ref", FTString, 16);
        shapefile_roads->add_field("type", FTString, 16);
        shapefile_roads->add_field("oneway", FTInteger, 1);
        shapefile_roads->add_field("bridge", FTInteger, 1);
        shapefile_roads->add_field("maxspeed", FTInteger, 3);

        fn.assign("railways");
        shapefile_railways = new Osmium::Export::LineStringShapefile(fn);
        shapefile_railways->add_field("osm_id", FTInteger, 11);
        shapefile_railways->add_field("name", FTString, 48);
        shapefile_railways->add_field("type", FTString, 16);

        fn.assign("points");
        shapefile_points = new Osmium::Export::PointShapefile(fn);
        shapefile_points->add_field("osm_id", FTInteger, 11);
        shapefile_points->add_field("timestamp", FTString, 20);
        shapefile_points->add_field("name", FTString, 48);
        shapefile_points->add_field("type", FTString, 16);

        fn.assign("places");
        shapefile_places = new Osmium::Export::PointShapefile(fn);
        shapefile_places->add_field("osm_id", FTInteger, 11);
        shapefile_places->add_field("name", FTString, 48);
        shapefile_places->add_field("type", FTString, 16);
        shapefile_places->add_field("population", FTInteger, 9);

        fn.assign("natural");
        shapefile_natural = new Osmium::Export::PolygonShapefile(fn);
        shapefile_natural->add_field("osm_id", FTInteger, 11);
        shapefile_natural->add_field("name", FTString, 48);
        shapefile_natural->add_field("type", FTString, 16);

        fn.assign("landuse");
        shapefile_landuse = new Osmium::Export::PolygonShapefile(fn);
        shapefile_landuse->add_field("osm_id", FTInteger, 11);
        shapefile_landuse->add_field("name", FTString, 48);
        shapefile_landuse->add_field("type", FTString, 16);

    }

    ~MyShapeHandler() {
    }

    void init(Osmium::OSM::Meta& meta) {
        handler_cfw->init(meta);
    }

    void node(Osmium::OSM::Node *node) {
        handler_cfw->node(node);
        const char *type = node->tags().get_tag_by_key("amenity");
        if (!type) type = node->tags().get_tag_by_key("tourism");
        if (!type) type = node->tags().get_tag_by_key("historic");
        if (!type) type = node->tags().get_tag_by_key("man_made");
        if (!type) type = node->tags().get_tag_by_key("railway");
        if (!type) type = node->tags().get_tag_by_key("highway");
        const char *name = node->tags().get_tag_by_key("name");
        const char *place = node->tags().get_tag_by_key("place");

        if (!place && !type) return;

        try {
            Osmium::Geometry::Point point(*node);
            if (place)
            {
                const char *pop = node->tags().get_tag_by_key("population");
                shapefile_places->add_geometry(point.create_shp_object());
                shapefile_places->add_attribute(0, node->id());
                if (name) shapefile_places->add_attribute_with_truncate(1, name);
                shapefile_places->add_attribute_with_truncate(2, type);
                if (pop) shapefile_places->add_attribute(3, atoi(pop));
            }
            else if (type)
            {
                shapefile_points->add_geometry(point.create_shp_object());
                shapefile_points->add_attribute(0, node->id());
                shapefile_points->add_attribute(1, node->timestamp_as_string());
                if (name) shapefile_points->add_attribute_with_truncate(2, name);
                shapefile_points->add_attribute_with_truncate(3, type);
            }
        } catch (Osmium::Exception::IllegalGeometry) {
            std::cerr << "Ignoring illegal geometry for node " << node->id() << ".\n";
        }
    }

    void after_nodes() {
        handler_cfw->after_nodes();
    }
    void after_ways() {
        delete shapefile_buildings;
        delete shapefile_waterways;
        delete shapefile_roads;
        delete shapefile_railways;
        delete shapefile_points;
        delete shapefile_places;
        delete shapefile_natural;
    }

    void way(Osmium::OSM::Way *way) {
        handler_cfw->way(way);
        
        try {
            const char *value = way->tags().get_tag_by_key("highway");
            const char *name = way->tags().get_tag_by_key("name");
            if (value)
            {
                const char *ref = way->tags().get_tag_by_key("ref");
                const char *oneway = way->tags().get_tag_by_key("oneway");
                int ow = 0;
                if (oneway)
                {
                    if (strcmp(oneway, "false") && strcmp(oneway, "no") && strcmp(oneway, "0"))
                    {
                        ow = 1;
                    }
                }
                const char *bridge = way->tags().get_tag_by_key("bridge");
                int br = 0;
                if (bridge)
                {
                    if (strcmp(bridge, "false") && strcmp(bridge, "no") && strcmp(bridge, "0"))
                    {
                        br = 1;
                    }
                }
                const char *maxspeed = way->tags().get_tag_by_key("maxspeed");

                Osmium::Geometry::LineString linestring(*way);
                shapefile_roads->add_geometry(linestring.create_shp_object());
                shapefile_roads->add_attribute(0, way->id());
                if (name) shapefile_roads->add_attribute_with_truncate(1, name);
                if (ref) shapefile_roads->add_attribute_with_truncate(2, ref);
                shapefile_roads->add_attribute_with_truncate(3, value);
                shapefile_roads->add_attribute(4, ow);
                shapefile_roads->add_attribute(5, br);
                shapefile_roads->add_attribute(6, maxspeed ? atoi(maxspeed) : 0);
                return;
            }

            value = way->tags().get_tag_by_key("building");
            if (value)
            {
                const char *type = strcmp(value, "yes") ? value : 0;
                if (!type) type = way->tags().get_tag_by_key("amenity");
                if (!type) type = way->tags().get_tag_by_key("tourism");
                if (!type) type = way->tags().get_tag_by_key("historic");
                if (!type) type = way->tags().get_tag_by_key("man_made");
                if (!type) type = way->tags().get_tag_by_key("railway");
                if (!type) type = way->tags().get_tag_by_key("highway");

                Osmium::Geometry::Polygon poly(*way);
                shapefile_buildings->add_geometry(poly.create_shp_object());
                shapefile_buildings->add_attribute(0, way->id());
                if (name) shapefile_buildings->add_attribute_with_truncate(1, name);
                if (type) shapefile_buildings->add_attribute_with_truncate(2, type);
                return;
            }

            value = way->tags().get_tag_by_key("railway");
            if (value)
            {
                Osmium::Geometry::LineString linestring(*way);
                shapefile_railways->add_geometry(linestring.create_shp_object());
                shapefile_railways->add_attribute(0, way->id());
                if (name) shapefile_railways->add_attribute_with_truncate(1, name);
                shapefile_railways->add_attribute_with_truncate(2, value);
                return;
            }

            value = way->tags().get_tag_by_key("waterway");
            if (value && strcmp(value, "riverbank"))
            {
                Osmium::Geometry::LineString linestring(*way);
                const char *width = way->tags().get_tag_by_key("width");
                shapefile_waterways->add_geometry(linestring.create_shp_object());
                shapefile_waterways->add_attribute(0, way->id());
                if (name) shapefile_waterways->add_attribute_with_truncate(1, name);
                if (value) shapefile_waterways->add_attribute_with_truncate(2, value);
                shapefile_waterways->add_attribute(3, width ? atoi(width) : 0);
                return;
            }

            if (value)
            {
                Osmium::Geometry::Polygon poly(*way);
                shapefile_natural->add_geometry(poly.create_shp_object());
                shapefile_natural->add_attribute(0, way->id());
                if (name) shapefile_natural->add_attribute_with_truncate(1, name);
                shapefile_natural->add_attribute(2, "riverbank");
                return;
            }

            const char *natural = way->tags().get_tag_by_key("natural");
            const char *landuse = way->tags().get_tag_by_key("landuse");
            const char *leisure = way->tags().get_tag_by_key("leisure");

            if ((landuse && !strcmp(landuse, "forest")) || (natural && !strcmp(natural, "wood")))
            {
                Osmium::Geometry::Polygon poly(*way);
                shapefile_natural->add_geometry(poly.create_shp_object());
                shapefile_natural->add_attribute(0, way->id());
                if (name) shapefile_natural->add_attribute_with_truncate(1, name);
                shapefile_natural->add_attribute(2, "forest");
                return;
            }
            if (leisure && !strcmp(leisure, "park"))
            {
                Osmium::Geometry::Polygon poly(*way);
                shapefile_natural->add_geometry(poly.create_shp_object());
                shapefile_natural->add_attribute(0, way->id());
                if (name) shapefile_natural->add_attribute_with_truncate(1, name);
                shapefile_natural->add_attribute(2, "park");
                return;
            }
            if (natural && !strcmp(natural, "water"))
            {
                Osmium::Geometry::Polygon poly(*way);
                shapefile_natural->add_geometry(poly.create_shp_object());
                shapefile_natural->add_attribute(0, way->id());
                if (name) shapefile_natural->add_attribute_with_truncate(1, name);
                shapefile_natural->add_attribute(2, "water");
                return;
            }
            if (landuse)
            {
                Osmium::Geometry::Polygon poly(*way);
                shapefile_landuse->add_geometry(poly.create_shp_object());
                shapefile_landuse->add_attribute(0, way->id());
                if (name) shapefile_landuse->add_attribute_with_truncate(1, name);
                shapefile_landuse->add_attribute_with_truncate(2, landuse);
                return;
            }
        } catch (Osmium::Exception::IllegalGeometry) {
            std::cerr << "Ignoring illegal geometry for way " << way->id() << ".\n";
        }
    }
};

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::init(true);

    int verbose=0;
    char *outdir = 0;

    while (1)
    {
        int c, option_index = 0;
        static struct option long_options[] = {
            {"verbose",  0, 0, 'v'},
            {"destination",  1, 0, 'd'},
            {"help",     0, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "hvd:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'v': verbose=1;  break;
            case 'd': outdir=optarg; break;
            case 'h':
            case '?':
            default:
                fprintf(stderr, "usage: %s [-d outdir] osmfile", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    char *oldwd = get_current_dir_name();
    if (outdir) chdir(outdir);
    MyShapeHandler handler;
    chdir(oldwd);
    while (optind < argc)
    {
        Osmium::OSMFile infile(argv[optind++]);
        infile.read(handler);
    }
}

