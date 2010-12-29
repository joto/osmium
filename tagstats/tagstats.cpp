
#include <osmium.hpp>

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

void node_handler(Osmium::OSM::Node *node) {
    osmium_handler_stats->callback_object(node);
    osmium_handler_tagstats->callback_object(node);
    osmium_handler_stats->callback_node(node);
//    osmium_handler_node_location_store->callback_node(node);
}

void after_nodes_handler() {
    osmium_handler_tagstats->callback_after_nodes();
}

void before_ways_handler() {
    osmium_handler_tagstats->callback_before_ways();
}

void way_handler(Osmium::OSM::Way *way) {
    osmium_handler_stats->callback_object(way);
    osmium_handler_tagstats->callback_object(way);
    osmium_handler_stats->callback_way(way);
//    osmium_handler_node_location_store->callback_way(way);
}

void after_ways_handler() {
    osmium_handler_tagstats->callback_after_ways();
}

void before_relations_handler() {
    osmium_handler_tagstats->callback_before_relations();
}

void relation_handler(Osmium::OSM::Relation *relation) {
    osmium_handler_stats->callback_object(relation);
    osmium_handler_tagstats->callback_object(relation);
    osmium_handler_stats->callback_relation(relation);
}

void after_relations_handler() {
    osmium_handler_tagstats->callback_after_relations();
}

void final_handler() {
    // osmium_handler_node_location_store->callback_final();
    osmium_handler_stats->callback_final();
    osmium_handler_tagstats->callback_final();
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
    bool debug = false; // XXX set this from command line

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    osmium_handler_stats               = new Osmium::Handler::Statistics(debug);
    osmium_handler_tagstats            = new Osmium::Handler::TagStats(debug);
//    osmium_handler_node_location_store = new Osmium::Handler::NLS_Sparsetable(debug);

    Osmium::OSM::Node     *node     = new Osmium::OSM::Node;
    Osmium::OSM::Way      *way      = new Osmium::OSM::Way;
    Osmium::OSM::Relation *relation = new Osmium::OSM::Relation;

    parse_osmfile(false, argv[1], setup_callbacks(), node, way, relation);

    return 0;
}

