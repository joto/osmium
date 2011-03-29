/*

  This is a small tool to dump the contents of the input file.

*/

#include <stdlib.h>
#include "osmium.hpp"
#include <HandlerDebug.hpp>

bool debug;
Osmium::Handler::Debug *osmium_handler_debug;

void init_handler() {
    std::cout << "init" << std::endl;
}

void before_nodes_handler() {
    std::cout << "before_nodes" << std::endl;
}

void node_handler(Osmium::OSM::Node *node) {
    osmium_handler_debug->callback_object(node);
    osmium_handler_debug->callback_node(node);
}

void after_nodes_handler() {
    std::cout << "after_nodes" << std::endl;
}

void before_ways_handler() {
    std::cout << "before_ways" << std::endl;
}

void way_handler(Osmium::OSM::Way *way) {
    osmium_handler_debug->callback_object(way);
    osmium_handler_debug->callback_way(way);
}

void after_ways_handler() {
    std::cout << "after_ways" << std::endl;
}

void before_relations_handler() {
    std::cout << "before_relations" << std::endl;
}

void relation_handler(Osmium::OSM::Relation *relation) {
    osmium_handler_debug->callback_object(relation);
    osmium_handler_debug->callback_relation(relation);
}

void after_relations_handler() {
    std::cout << "after_relations" << std::endl;
}

void final_handler() {
    std::cout << "final" << std::endl;
}


struct callbacks *setup_callbacks() {
    static struct callbacks cb;
    cb.init             = init_handler;
    cb.before_nodes     = before_nodes_handler;
    cb.node             = node_handler;
    cb.after_nodes      = after_nodes_handler;
    cb.before_ways      = before_ways_handler;
    cb.way              = way_handler;
    cb.after_ways       = after_ways_handler;
    cb.before_relations = before_relations_handler;
    cb.relation         = relation_handler;
    cb.after_relations  = after_relations_handler;
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

    osmium_handler_debug = new Osmium::Handler::Debug();

    parse_osmfile(argv[1], setup_callbacks());

    return 0;
}

