
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>

#include "osmium.hpp"

Osmium::Handler::NodeLocationStore *osmium_handler_node_location_store;
Osmium::Handler::Multipolygon      *osmium_handler_multipolygon;
Osmium::Handler::Javascript        *osmium_handler_javascript;
//Osmium::Handler::Bbox              *osmium_handler_bbox;

void single_pass_init_handler() {
    osmium_handler_node_location_store->callback_init();
    osmium_handler_javascript->callback_init();
}

void single_pass_node_handler(Osmium::OSM::Node *node) {
    osmium_handler_node_location_store->callback_node(node);
    osmium_handler_javascript->callback_node(node);
//    osmium_handler_bbox->callback_node(node);
}

void single_pass_way_handler(Osmium::OSM::Way *way) {
    osmium_handler_node_location_store->callback_way(way);
    osmium_handler_javascript->callback_way(way);
}

void single_pass_relation_handler(Osmium::OSM::Relation *relation) {
    osmium_handler_javascript->callback_relation(relation);
}

void single_pass_final_handler() {
//    osmium_handler_bbox->callback_final();
    osmium_handler_javascript->callback_final();
}

struct callbacks *setup_callbacks_single_pass() {
    static struct callbacks cb;
    cb.init             = single_pass_init_handler;
    cb.node             = single_pass_node_handler;
    cb.way              = single_pass_way_handler;
    cb.relation         = single_pass_relation_handler;
    cb.final            = single_pass_final_handler;
    return &cb;
}

void dual_pass_init_handler() {
    osmium_handler_node_location_store->callback_init();
    osmium_handler_javascript->callback_init();
}

void dual_pass_node_handler1(Osmium::OSM::Node *node) {
    osmium_handler_javascript->callback_node(node);
}

void dual_pass_before_relations_handler1() {
    osmium_handler_multipolygon->callback_before_relations();
}

void dual_pass_relation_handler1(Osmium::OSM::Relation *relation) {
    osmium_handler_multipolygon->callback_relation(relation);
    osmium_handler_javascript->callback_relation(relation);
}

void dual_pass_after_relations_handler1() {
    osmium_handler_multipolygon->callback_after_relations();
}

void dual_pass_node_handler2(Osmium::OSM::Node *node) {
    osmium_handler_node_location_store->callback_node(node);
}

void dual_pass_way_handler2(Osmium::OSM::Way *way) {
    osmium_handler_node_location_store->callback_way(way);
    osmium_handler_multipolygon->callback_way(way);
    osmium_handler_javascript->callback_way(way);
}

void dual_pass_after_ways_handler2() {
    osmium_handler_multipolygon->callback_after_ways();
}

void dual_pass_multipolygon_handler(Osmium::OSM::Multipolygon *multipolygon) {
    Osmium::Javascript::Multipolygon::Wrapper *mpw = new Osmium::Javascript::Multipolygon::Wrapper(multipolygon);
    osmium_handler_javascript->callback_multipolygon(multipolygon);
    delete mpw;
}

void dual_pass_final_handler() {
    osmium_handler_javascript->callback_final();
    osmium_handler_multipolygon->callback_final();
}
struct callbacks *setup_callbacks_1st_pass() {
    static struct callbacks cb;
    cb.init             = dual_pass_init_handler;
    cb.node             = dual_pass_node_handler1;
    cb.before_relations = dual_pass_before_relations_handler1;
    cb.relation         = dual_pass_relation_handler1;
    cb.after_relations  = dual_pass_after_relations_handler1;
    return &cb;
}

struct callbacks *setup_callbacks_2nd_pass() {
    static struct callbacks cb;
    cb.node             = dual_pass_node_handler2;
    cb.way              = dual_pass_way_handler2;
    cb.after_ways       = dual_pass_after_ways_handler2;
    cb.multipolygon     = dual_pass_multipolygon_handler;
    cb.final            = dual_pass_final_handler;
    return &cb;
}

/* ================================================== */

v8::Persistent<v8::Context> global_context;

void print_help() {
    std::cout << "osmjs [OPTIONS] OSMFILE" << std::endl \
              << "Options:" << std::endl \
              << "  --help, -h                       - This help message" << std::endl \
              << "  --debug, -d                      - Enable debugging output" << std::endl \
              << "  --include=FILE, -i FILE          - Include Javascript file (can be given several times)" << std::endl \
              << "  --javascript=FILE, -j FILE       - Process given Javascript file" << std::endl \
              << "  --location-store=STORE, -l STORE - Set location store (default: 'sparsetable')" << std::endl \
              << "  --no-repair, -r                  - Do not attempt to repair broken multipolygons" << std::endl \
              << "  --2pass, -2                      - Read OSMFILE twice and build multipolygons" << std::endl \
              << "Location stores:" << std::endl \
              << "  array       - Store node locations in large array (use for large OSM files)" << std::endl \
              << "  sparsetable - Store node locations in sparse table (use for small OSM files)" << std::endl;

}

std::string find_include_file(std::string filename) {
    static std::vector<std::string> search_path = {
        ".",
        "js",
        std::string(getenv("HOME")) + "/.osmjs",
        "/usr/local/share/osmjs",
        "/usr/share/osmjs"
    };

    // add .js if there is no suffix
    if (filename.find_last_of('.') == std::string::npos) {
        filename.append(".js");
    }

    // go through search path and find where the file is
    for (std::vector<std::string>::iterator vi = search_path.begin(); vi != search_path.end(); vi++) {
        std::string f = *vi + "/" + filename;
        if (!access(f.c_str(), R_OK)) {
            return f;
        }
    }

    std::cerr << "Could not find include file " << filename << " in search path (";
    for (std::vector<std::string>::iterator vi = search_path.begin(); vi != search_path.end(); vi++) {
        if (vi != search_path.begin()) std::cerr << ":";
        std::cerr << *vi;
    }
    std::cerr << ")" << std::endl;
    exit(1);
}

int main(int argc, char *argv[]) {
    bool debug = false;
    bool two_passes = false;
    bool attempt_repair = true;
    char javascript_filename[512] = "";
    char *osm_filename;
    std::vector<std::string> include_files;
    enum location_store_t {
        ARRAY,
        SPARSETABLE
    } location_store = SPARSETABLE;

    static struct option long_options[] = {
        {"debug",                no_argument, 0, 'd'},
        {"include",        required_argument, 0, 'i'},
        {"javascript",     required_argument, 0, 'j'},
        {"help",                 no_argument, 0, 'h'},
        {"location-store", required_argument, 0, 'l'},
        {"no-repair",            no_argument, 0, 'r'},
        {"2pass",                no_argument, 0, '2'},
        {0, 0, 0, 0}
    };

    while (1) {
        int c = getopt_long(argc, argv, "dhi:j:l:r2", long_options, 0);
        if (c == -1)
            break;

        switch (c) {
            case 'd':
                debug = true;
                break;
            case 'i': {
                std::string f(optarg);
                include_files.push_back(find_include_file(f));
                break;
            }
            case 'j':
                strncpy(javascript_filename, optarg, 512);
                break;
            case 'h':
                print_help();
                exit(0);
            case 'l':
                if (!strcmp(optarg, "array")) {
                    location_store = ARRAY;
                } else if (!strcmp(optarg, "sparsetable")) {
                    location_store = SPARSETABLE;
                } else {
                    std::cerr << "Unknown location store: " << optarg << " (available are: 'array' and 'sparsetable')" << std::endl;
                    exit(1);
                }
                break;
            case 'r':
                attempt_repair = false;
                break;
            case '2':
                two_passes = true;
                break;
            default:
                exit(1);
        }
    }

    if (javascript_filename[0] == '\0') {
        std::cerr << "No --javascript/-j option given" << std::endl;
        exit(1);
    }

    if (optind == argc-1) {
        osm_filename = argv[optind];
    } else {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] OSMFILE" << std::endl;
        exit(1);
    }

    v8::HandleScope handle_scope;

    v8::Handle<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New();
    global_template->Set(v8::String::New("print"), v8::FunctionTemplate::New(Osmium::Handler::Javascript::Print));
    global_template->Set(v8::String::New("include"), v8::FunctionTemplate::New(Osmium::Handler::Javascript::Include));

    global_context = v8::Persistent<v8::Context>::New(v8::Context::New(0, global_template));
    v8::Context::Scope context_scope(global_context);

    Osmium::Javascript::Template::init();

    struct callbacks *callbacks_single_pass = setup_callbacks_single_pass();
    struct callbacks *callbacks_1st_pass    = setup_callbacks_1st_pass();
    struct callbacks *callbacks_2nd_pass    = setup_callbacks_2nd_pass();

    if (location_store == ARRAY) {
        osmium_handler_node_location_store = new Osmium::Handler::NLS_Array(debug);
    } else {
        osmium_handler_node_location_store = new Osmium::Handler::NLS_Sparsetable(debug);
    }
    osmium_handler_javascript = new Osmium::Handler::Javascript(debug, include_files, javascript_filename);
    if (two_passes) {
        osmium_handler_multipolygon = new Osmium::Handler::Multipolygon(debug, attempt_repair, callbacks_2nd_pass);
    }

    Osmium::Javascript::Node::Wrapper     *wrap_node     = new Osmium::Javascript::Node::Wrapper;
    Osmium::Javascript::Way::Wrapper      *wrap_way      = new Osmium::Javascript::Way::Wrapper;
    Osmium::Javascript::Relation::Wrapper *wrap_relation = new Osmium::Javascript::Relation::Wrapper;

    Osmium::OSM::Node     *node     = wrap_node->object;
    Osmium::OSM::Way      *way      = wrap_way->object;
    Osmium::OSM::Relation *relation = wrap_relation->object;

    if (two_passes) {
        parse_osmfile(debug, osm_filename, callbacks_1st_pass,    node, way, relation);
        parse_osmfile(debug, osm_filename, callbacks_2nd_pass,    node, way, relation);
    } else {
        parse_osmfile(debug, osm_filename, callbacks_single_pass, node, way, relation);
    }

    delete wrap_relation;
    delete wrap_way;
    delete wrap_node;

    if (two_passes) {
        delete osmium_handler_multipolygon;
    }
    delete osmium_handler_javascript;
    delete osmium_handler_node_location_store;
	
    global_context.Dispose();

    return 0;
}

