/*

  This is a small tool to try the Statistics handler

*/

#include <stdlib.h>
#include "osmium.hpp"
#include <HandlerStatistics.hpp>

bool debug;
Osmium::Handler::Statistics *osmium_handler_statistics;

void node_handler(Osmium::OSM::Node *node) {
    osmium_handler_statistics->callback_object(node);
    osmium_handler_statistics->callback_node(node);
}

void way_handler(Osmium::OSM::Way *way) {
    osmium_handler_statistics->callback_object(way);
    osmium_handler_statistics->callback_way(way);
}

void relation_handler(Osmium::OSM::Relation *relation) {
    osmium_handler_statistics->callback_object(relation);
    osmium_handler_statistics->callback_relation(relation);
}

void final_handler() {
    osmium_handler_statistics->callback_final();
}


struct callbacks *setup_callbacks() {
    static struct callbacks cb;
    cb.init             = NULL;
    cb.before_nodes     = NULL;
    cb.node             = node_handler;
    cb.after_nodes      = NULL;
    cb.before_ways      = NULL;
    cb.way              = way_handler;
    cb.after_ways       = NULL;
    cb.before_relations = NULL;
    cb.relation         = relation_handler;
    cb.after_relations  = NULL;
    cb.final            = final_handler;
    return &cb;
}

/* ================================================== */

int main(int argc, char *argv[]) {
    debug = true;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    osmium_handler_statistics = new Osmium::Handler::Statistics();

    parse_osmfile(argv[1], setup_callbacks());

    return 0;
}

