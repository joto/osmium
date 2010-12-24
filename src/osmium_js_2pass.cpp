
#include "osmium.hpp"

Osmium::Handler::NLS_Sparsetable *osmium_handler_node_location_store;
Osmium::Handler::Multipolygon    *osmium_handler_multipolygon;
Osmium::Handler::Javascript      *osmium_handler_javascript;

void init_handler() {
    osmium_handler_node_location_store->callback_init();
    osmium_handler_javascript->callback_init();
}

void node_handler1(Osmium::OSM::Node *node) {
    osmium_handler_javascript->callback_node(node);
}

void before_relations_handler1() {
    osmium_handler_multipolygon->callback_before_relations();
}

void relation_handler1(Osmium::OSM::Relation *relation) {
    osmium_handler_multipolygon->callback_relation(relation);
    osmium_handler_javascript->callback_relation(relation);
}

void after_relations_handler1() {
    osmium_handler_multipolygon->callback_after_relations();
}

void node_handler2(Osmium::OSM::Node *node) {
    osmium_handler_node_location_store->callback_node(node);
}

void way_handler2(Osmium::OSM::Way *way) {
    osmium_handler_node_location_store->callback_way(way);
    osmium_handler_multipolygon->callback_way(way);
    osmium_handler_javascript->callback_way(way);
}

void after_ways_handler2() {
    osmium_handler_multipolygon->callback_after_ways();
}

void multipolygon_handler(Osmium::OSM::Multipolygon *multipolygon) {
    std::cerr << " MP HANDLER " << multipolygon->get_id() << "\n";

    Osmium::Javascript::Multipolygon::Wrapper *mpw = new Osmium::Javascript::Multipolygon::Wrapper(multipolygon);
    osmium_handler_javascript->callback_multipolygon(multipolygon);
    delete mpw;
}

void final_handler() {
    osmium_handler_node_location_store->callback_final();
    osmium_handler_javascript->callback_final();
    osmium_handler_multipolygon->callback_final();
}

struct callbacks *setup_callbacks_1st_pass() {
    static struct callbacks cb;
    cb.init             = init_handler;
    cb.node             = node_handler1;
    cb.before_relations = before_relations_handler1;
    cb.relation         = relation_handler1;
    cb.after_relations  = after_relations_handler1;
    return &cb;
}

struct callbacks *setup_callbacks_2nd_pass() {
    static struct callbacks cb;
    cb.node             = node_handler2;
    cb.way              = way_handler2;
    cb.after_ways       = after_ways_handler2;
    cb.multipolygon     = multipolygon_handler;
    cb.final            = final_handler;
    return &cb;
}

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
    global_template->Set(v8::String::New("include"), v8::FunctionTemplate::New(Osmium::Handler::Javascript::Include));

    global_context = v8::Persistent<v8::Context>::New(v8::Context::New(0, global_template));
    v8::Context::Scope context_scope(global_context);

    Osmium::Javascript::Template::init();

    struct callbacks *callbacks_1st_pass = setup_callbacks_1st_pass();
    struct callbacks *callbacks_2nd_pass = setup_callbacks_2nd_pass();

    osmium_handler_node_location_store = new Osmium::Handler::NLS_Sparsetable;
    osmium_handler_multipolygon        = new Osmium::Handler::Multipolygon(callbacks_2nd_pass);
    osmium_handler_javascript          = new Osmium::Handler::Javascript(argv[1]);

    Osmium::Javascript::Node::Wrapper     *wrap_node     = new Osmium::Javascript::Node::Wrapper;
    Osmium::Javascript::Way::Wrapper      *wrap_way      = new Osmium::Javascript::Way::Wrapper;
    Osmium::Javascript::Relation::Wrapper *wrap_relation = new Osmium::Javascript::Relation::Wrapper;

    Osmium::OSM::Node     *node     = wrap_node->object;
    Osmium::OSM::Way      *way      = wrap_way->object;
    Osmium::OSM::Relation *relation = wrap_relation->object;

    if (argv[2][0] == '-') {
        std::cerr << "Two pass handler can´t read from stdin\n";
    }

    parse_osmfile(argv[2], callbacks_1st_pass, node, way, relation);
    parse_osmfile(argv[2], callbacks_2nd_pass, node, way, relation);
	
    global_context.Dispose();

    return 0;
}

