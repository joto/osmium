#ifndef OSMIUM_OSMIUM_HPP
#define OSMIUM_OSMIUM_HPP

#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>
#include <assert.h>

#include "wkb.hpp"
#include "StringStore.hpp"

/// OSM file format
enum osm_file_format_t {
    xml, ///< XML format (see http://wiki.openstreetmap.org/wiki/Data_Primitives)
    pbf  ///< PBF format (see http://wiki.openstreetmap.org/wiki/PBF_Format)
};

enum osm_object_type_t {
    NODE     = 1,
    WAY      = 2,
    RELATION = 3
};
#define OSM_OBJECT_TYPES_COUNT 4

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

#include "wkb.hpp"

#include "Osm.hpp"

struct callbacks {
    void (*before_nodes)();
    void (*after_nodes)();
    void (*before_ways)();
    void (*after_ways)();
    void (*before_relations)();
    void (*after_relations)();
    void (*node)(Osmium::OSM::Node *node);
    void (*way)(Osmium::OSM::Way *way);
    void (*relation)(Osmium::OSM::Relation *relation);
};

#include "Handler.hpp"
#include "HandlerStatistics.hpp"
#include "HandlerTagStats.hpp"
#include "HandlerNodeLocationStore.hpp"

#include "JavascriptOutputCSV.hpp"
#include "JavascriptOutputShapefile.hpp"

#include "HandlerJavascript.hpp"

#include "Javascript.hpp"

#endif // OSMIUM_OSMIUM_HPP
