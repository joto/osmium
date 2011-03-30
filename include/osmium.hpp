#ifndef OSMIUM_OSMIUM_HPP
#define OSMIUM_OSMIUM_HPP

#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>
#include <assert.h>

#include <google/protobuf/stubs/common.h>

#include "wkb.hpp"
#include "StringStore.hpp"

/// OSM file format
enum osm_file_format_t {
    xml, ///< XML format (see http://wiki.openstreetmap.org/wiki/Data_Primitives)
    pbf  ///< PBF format (see http://wiki.openstreetmap.org/wiki/PBF_Format)
};

enum osm_object_type_t {
    NODE                       = 1,
    WAY                        = 2,
    RELATION                   = 3,
    MULTIPOLYGON_FROM_WAY      = 4,
    MULTIPOLYGON_FROM_RELATION = 5
};

#define OSM_OBJECT_ID_SIZE sizeof(osm_object_id_t)

#define STR_TO_OBJECT_ID(x)    (atol(x))
#define STR_TO_VERSION(x)      (atoi(x))
#define STR_TO_CHANGESET_ID(x) (atol(x))
#define STR_TO_USER_ID(x)      (atol(x))
#define STR_TO_SEQUENCE_ID(x)  (atoi(x))

struct node_coordinates {
    double lon;
    double lat;
};

// check way geometry before making a shplib object from it
// normally this should be defined, otherwise you will generate invalid linestring geometries
#define CHECK_WAY_GEOMETRY

#ifdef WITH_GEOS
#include <geos/geom/GeometryFactory.h>

namespace Osmium {
    geos::geom::GeometryFactory *geos_factory();
} // namespace Osmium

#endif

#include "wkb.hpp"

#include "Osm.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Input.hpp"
#include "XMLParser.hpp"
#include "PBFParser.hpp"

/**
*
*  Parse OSM file and call callback functions.
*  This works for OSM XML files (suffix .osm) and OSM binary files (suffix .pbf).
*  Reads from STDIN if the filename is '-', in this case it assumes XML format.
*
*/
template <class T> void parse_osmfile(char *osmfilename, T* handler = NULL) {
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

    Osmium::Input::Base<T> *input;
    switch (file_format) {
        case xml:
            input = new Osmium::Input::XML<T>(fd, handler);
            break;
        case pbf:
            input = new Osmium::Input::PBF<T>(fd, handler);
            break;
    }
    input->parse();
    delete input;

    close(fd);
}


#include "Handler.hpp"
#include "HandlerBbox.hpp"
#include "HandlerMultipolygon.hpp"
#include "HandlerNodeLocationStore.hpp"
#include "HandlerStatistics.hpp"
#include "HandlerTagStats.hpp"

#ifdef WITH_JAVASCRIPT
# include "JavascriptOutputCSV.hpp"
# include "JavascriptOutputShapefile.hpp"
# include "HandlerJavascript.hpp"
# include "Javascript.hpp"
#endif

#endif // OSMIUM_OSMIUM_HPP
