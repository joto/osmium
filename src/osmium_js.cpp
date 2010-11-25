
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "osmium.hpp"
#include "XMLParser.hpp"
#include "Javascript.hpp"

Osmium::Handler::NLS_Sparsetable *osmium_handler_node_location_store;
Osmium::Handler::Javascript      *osmium_handler_javascript;

namespace Osmium {

    void init_handler() {
        osmium_handler_node_location_store->callback_init();
        osmium_handler_javascript->callback_init();
    }

    void before_nodes_handler() {
    }

    void after_nodes_handler() {
    }

    void before_ways_handler() {
    }

    void after_ways_handler() {
    }

    void before_relations_handler() {
    }

    void after_relations_handler() {
    }

    void final_handler() {
        osmium_handler_node_location_store->callback_final();
        osmium_handler_javascript->callback_final();
    }

    void object_handler(OSM::Object *object) {
        switch (object->type()) {
            case NODE:
                osmium_handler_node_location_store->callback_node((OSM::Node *) object);
                osmium_handler_javascript->callback_node((OSM::Node *) object);
                break;
            case WAY:
                osmium_handler_node_location_store->callback_way((OSM::Way *) object);
                osmium_handler_javascript->callback_way((OSM::Way *) object);
                break;
            case RELATION:
                osmium_handler_javascript->callback_relation((OSM::Relation *) object);
                break;
        }
    }

} // namespace Osmium

/* ================================================== */

v8::Persistent<v8::Context> global_context;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " JAVASCRIPTFILE OSMFILE\n";
        exit(1);
    }

    v8::HandleScope handle_scope;

    v8::Handle<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New();
    global_template->Set(v8::String::New("print"), v8::FunctionTemplate::New(Osmium::Handler::Javascript::Print));

    global_context = v8::Persistent<v8::Context>::New(v8::Context::New(0, global_template));
    v8::Context::Scope context_scope(global_context);

    Osmium::Javascript::Template::init();

    osmium_handler_node_location_store = new Osmium::Handler::NLS_Sparsetable;
    osmium_handler_javascript          = new Osmium::Handler::Javascript(argv[1]);

    Osmium::Javascript::Node::Wrapper     *wrap_node     = new Osmium::Javascript::Node::Wrapper;
    Osmium::Javascript::Way::Wrapper      *wrap_way      = new Osmium::Javascript::Way::Wrapper;
    Osmium::Javascript::Relation::Wrapper *wrap_relation = new Osmium::Javascript::Relation::Wrapper;

    Osmium::OSM::Node     *node     = wrap_node->object;
    Osmium::OSM::Way      *way      = wrap_way->object;
    Osmium::OSM::Relation *relation = wrap_relation->object;

//    Osmium::OSM::Node     *node     = new Osmium::OSM::Node;
//    Osmium::OSM::Way      *way      = new Osmium::OSM::Way;
//    Osmium::OSM::Relation *relation = new Osmium::OSM::Relation;

    const char *filename = argv[2];
    int fd = 0;
    if (filename[0] == '-' && filename[1] == 0) {
        // fd is already 0, read STDIN
    } else {
        fd = open(filename, O_RDONLY);
    }

    bool parseok = Osmium::XMLParser::parse(fd, node, way, relation);
    close(fd);

    if (! parseok) {
        std::cerr << "Error occurred while parsing: " << filename << std::endl;
    }
	
    global_context.Dispose();

    return 0;
}

