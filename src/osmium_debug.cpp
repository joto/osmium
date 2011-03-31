/*

  This is a small tool to dump the contents of the input file.

*/

#include <cstdlib>

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>

bool debug;

class MyDebugHandler : public Osmium::Handler::Base {

    Osmium::Handler::Debug handler_debug;

  public:

    void callback_init() {
        std::cout << "init" << std::endl;
    }

    void callback_object(Osmium::OSM::Object *object) {
        handler_debug.callback_object(object);
    }

    void callback_before_nodes() {
        std::cout << "before_nodes" << std::endl;
    }

    void callback_node(Osmium::OSM::Node *node) {
        handler_debug.callback_node(node);
    }

    void callback_after_nodes() {
        std::cout << "after_nodes" << std::endl;
    }

    void callback_before_ways() {
        std::cout << "before_ways" << std::endl;
    }

    void callback_way(Osmium::OSM::Way *way) {
        handler_debug.callback_way(way);
    }

    void callback_after_ways() {
        std::cout << "after_ways" << std::endl;
    }

    void callback_before_relations() {
        std::cout << "before_relations" << std::endl;
    }

    void callback_relation(Osmium::OSM::Relation *relation) {
        handler_debug.callback_relation(relation);
    }

    void callback_after_relations() {
        std::cout << "after_relations" << std::endl;
    }

    void callback_final() {
        std::cout << "final" << std::endl;
    }
};

/* ================================================== */

int main(int argc, char *argv[]) {
    debug = true;
    Osmium::Framework osmium;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    osmium.parse_osmfile<MyDebugHandler>(argv[1]);
}

