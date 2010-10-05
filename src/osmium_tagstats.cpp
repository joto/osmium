
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "osmium.hpp"
#include "XMLParser.hpp"

Osmium::Handler::Statistics      *osmium_handler_stats;
Osmium::Handler::TagStats        *osmium_handler_tagstats;
Osmium::Handler::NLS_Sparsetable *osmium_handler_node_location_store;

namespace Osmium {

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

    void object_handler(OSM::Object *object) {
        osmium_handler_stats->callback_object(object);
        osmium_handler_tagstats->callback_object(object);
        switch (object->type()) {
            case NODE:
                osmium_handler_stats->callback_node((OSM::Node *) object);
                osmium_handler_node_location_store->callback_node((OSM::Node *) object);
                break;
            case WAY:
                osmium_handler_stats->callback_way((OSM::Way *) object);
                osmium_handler_node_location_store->callback_way((OSM::Way *) object);
                break;
            case RELATION:
                osmium_handler_stats->callback_relation((OSM::Relation *) object);
                break;
        }
    }

} // namespace Osmium

/* ================================================== */

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    osmium_handler_stats               = new Osmium::Handler::Statistics;
    osmium_handler_tagstats            = new Osmium::Handler::TagStats;
    osmium_handler_node_location_store = new Osmium::Handler::NLS_Sparsetable;

    Osmium::OSM::Node     *node     = new Osmium::OSM::Node;
    Osmium::OSM::Way      *way      = new Osmium::OSM::Way;
    Osmium::OSM::Relation *relation = new Osmium::OSM::Relation;

    const char *osmfilename = argv[1];
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

    bool parseok = Osmium::XMLParser::parse(fd, node, way, relation);
    close(fd);

    if (! parseok) {
        std::cerr << "Error occurred while parsing: " << osmfilename << '\n';
    }
	
    return 0;
}

