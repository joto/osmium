
#include "osmium.hpp"
#include "Input.hpp"
#include "XMLParser.hpp"
#include "PBFParser.hpp"

#ifdef WITH_GEOS
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

extern bool debug;

namespace Osmium {
    geos::geom::GeometryFactory *geos_factory() {
        static geos::geom::GeometryFactory *global_geometry_factory;

        if (! global_geometry_factory) {
            geos::geom::PrecisionModel *pm = new geos::geom::PrecisionModel();
            global_geometry_factory = new geos::geom::GeometryFactory(pm, -1);
        }

        return global_geometry_factory;
    }
} // namespace Osmium

#endif

// static buffers
char Osmium::OSM::Node::lon_str[];
char Osmium::OSM::Node::lat_str[];

// needed for writing WKB (see wkb.hpp)
extern const char lookup_hex[] = "0123456789abcdef";

#ifdef WITH_MULTIPOLYGON_PROFILING
std::vector<std::pair<std::string, timer *> > Osmium::OSM::MultipolygonFromRelation::timers;

timer Osmium::OSM::MultipolygonFromRelation::write_complex_poly_timer;
timer Osmium::OSM::MultipolygonFromRelation::assemble_ways_timer;
timer Osmium::OSM::MultipolygonFromRelation::assemble_nodes_timer;
timer Osmium::OSM::MultipolygonFromRelation::make_one_ring_timer;
timer Osmium::OSM::MultipolygonFromRelation::mor_polygonizer_timer;
timer Osmium::OSM::MultipolygonFromRelation::mor_union_timer;
timer Osmium::OSM::MultipolygonFromRelation::contains_timer;
timer Osmium::OSM::MultipolygonFromRelation::extra_polygons_timer;
timer Osmium::OSM::MultipolygonFromRelation::polygon_build_timer;
timer Osmium::OSM::MultipolygonFromRelation::inner_ring_touch_timer;
timer Osmium::OSM::MultipolygonFromRelation::polygon_intersection_timer;
timer Osmium::OSM::MultipolygonFromRelation::multipolygon_build_timer;
timer Osmium::OSM::MultipolygonFromRelation::multipolygon_write_timer;
timer Osmium::OSM::MultipolygonFromRelation::error_write_timer;
#endif

#include <fcntl.h>

/**
*
*  Parse OSM file and call callback functions.
*  This works for OSM XML files (suffix .osm) and OSM binary files (suffix .pbf).
*  Reads from STDIN if the filename is '-', in this case it assumes XML format.
*
*/
void parse_osmfile(char *osmfilename, struct callbacks *callbacks, Osmium::OSM::Node *node, Osmium::OSM::Way *way, Osmium::OSM::Relation *relation) {
    int fd = 0;
    if (osmfilename[0] == '-' && osmfilename[1] == '\0') {
        // fd is already 0, read STDIN
    } else {
        fd = open(osmfilename, O_RDONLY);
        if (fd < 0) {
            std::cerr << "Can't open osm file: " << strerror(errno) << std::endl;
            exit(1);
        }
    }

    osm_file_format_t file_format;
    char *suffix = strrchr(osmfilename, '.');

    if (suffix == NULL) {
        file_format = xml;
    } else {
        if (!strcmp(suffix, ".osm")) {
            file_format = xml;
        } else if (!strcmp(suffix, ".pbf")) {
            file_format = pbf;
        } else {
            std::cerr << "Unknown file suffix: " << suffix << std::endl;
            exit(1);
        }
    }

    if (callbacks->init) { callbacks->init(); }
    Osmium::Input::Base *input;
    switch (file_format) {
        case xml:
            input = new Osmium::Input::XML(fd, callbacks);
            break;
        case pbf:
            input = new Osmium::Input::PBF(fd, callbacks);
            break;
    }
    input->parse(node, way, relation);
    delete input;
    if (callbacks->final) { callbacks->final(); }

    close(fd);
}


