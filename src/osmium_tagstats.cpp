
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "osmium.hpp"
#include "XMLParser.hpp"
#include "PBFParser.hpp"

Osmium::Handler::Statistics      *osmium_handler_stats;
Osmium::Handler::TagStats        *osmium_handler_tagstats;
//Osmium::Handler::NLS_Sparsetable *osmium_handler_node_location_store;

void init_handler() {
    // osmium_handler_stats->callback_init();
    osmium_handler_tagstats->callback_init();
    // osmium_handler_node_location_store->callback_init();
}

void before_nodes_handler() {
    osmium_handler_tagstats->callback_before_nodes();
}

void after_nodes_handler() {
    osmium_handler_tagstats->callback_after_nodes();
}

void before_ways_handler() {
    osmium_handler_tagstats->callback_before_ways();
}

void after_ways_handler() {
    osmium_handler_tagstats->callback_after_ways();
}

void before_relations_handler() {
    osmium_handler_tagstats->callback_before_relations();
}

void after_relations_handler() {
    osmium_handler_tagstats->callback_after_relations();
}

void final_handler() {
    // osmium_handler_node_location_store->callback_final();
    osmium_handler_stats->callback_final();
    osmium_handler_tagstats->callback_final();
}

void node_handler(Osmium::OSM::Node *object) {
    osmium_handler_stats->callback_object(object);
    osmium_handler_tagstats->callback_object(object);
    osmium_handler_stats->callback_node((Osmium::OSM::Node *) object);
//    osmium_handler_node_location_store->callback_node((Osmium::OSM::Node *) object);
}

void way_handler(Osmium::OSM::Way *object) {
    osmium_handler_stats->callback_object(object);
    osmium_handler_tagstats->callback_object(object);
    osmium_handler_stats->callback_way((Osmium::OSM::Way *) object);
//    osmium_handler_node_location_store->callback_way((Osmium::OSM::Way *) object);
}

void relation_handler(Osmium::OSM::Relation *object) {
    osmium_handler_stats->callback_object(object);
    osmium_handler_tagstats->callback_object(object);
    osmium_handler_stats->callback_relation((Osmium::OSM::Relation *) object);
}


struct callbacks callbacks = {
    before_nodes_handler,
    after_nodes_handler,
    before_ways_handler,
    after_ways_handler,
    before_relations_handler,
    after_relations_handler,
    node_handler,
    way_handler,
    relation_handler
};

/* ================================================== */

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    osmium_handler_stats               = new Osmium::Handler::Statistics;
    osmium_handler_tagstats            = new Osmium::Handler::TagStats;
//    osmium_handler_node_location_store = new Osmium::Handler::NLS_Sparsetable;

    Osmium::OSM::Node     *node     = new Osmium::OSM::Node;
    Osmium::OSM::Way      *way      = new Osmium::OSM::Way;
    Osmium::OSM::Relation *relation = new Osmium::OSM::Relation;

    char *osmfilename = argv[1];
    int fd = 0;
    if (osmfilename[0] == '-' && osmfilename[1] == 0) {
        // fd is already 0, read STDIN
    } else {
        fd = open(osmfilename, O_RDONLY);
        if (fd < 0) {
            std::cerr << "Can't open osm file: " << strerror(errno) << '\n';
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
            std::cerr << "Unknown file suffix: " << suffix << "\n";
            exit(1);
        }
    }

    init_handler();
    switch (file_format) {
        case xml:
            Osmium::XMLParser::parse(fd, &callbacks, node, way, relation);
            break;
        case pbf:
            Osmium::PBFParser *pbf_parser = new Osmium::PBFParser(fd, &callbacks);
            pbf_parser->parse(node, way, relation);
            break;
    }
    final_handler();

    close(fd);

    return 0;
}

