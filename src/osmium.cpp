
#include "osmium.hpp"
#include "XMLParser.hpp"
#include "PBFParser.hpp"

#ifdef WITH_GEOS
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

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

#include <fcntl.h>

/**
*
*  Parse OSM file and call callback functions.
*  This works for OSM XML files (suffix .osm) and OSM binary files (suffix .pbf).
*  Reads from STDIN if the filename is '-', in this case it assumes XML format.
*
*/
void parse_osmfile(bool debug, char *osmfilename, struct callbacks *callbacks, Osmium::OSM::Node *node, Osmium::OSM::Way *way, Osmium::OSM::Relation *relation, Osmium::OSM::Multipolygon *multipolygon) {
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
    switch (file_format) {
        case xml:
            Osmium::XMLParser::parse(fd, callbacks, node, way, relation, multipolygon);
            break;
        case pbf:
            Osmium::PBFParser *pbf_parser = new Osmium::PBFParser(debug, fd, callbacks);
            pbf_parser->parse(node, way, relation, multipolygon);
            delete pbf_parser;
            break;
    }
    if (callbacks->final) { callbacks->final(); }

    close(fd);
}


