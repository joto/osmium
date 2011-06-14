
#include <cstdlib>
#include <getopt.h>
#include <unistd.h>

#define OSMIUM_MAIN
#include <osmium.hpp>
#include <osmium/handler/node_location_store.hpp>
#include <osmium/handler/multipolygon.hpp>

#ifdef OSMIUM_WITH_MULTIPOLYGON_PROFILING
std::vector<std::pair<std::string, timer *> > Osmium::OSM::MultipolygonFromRelation::timers;

timer Osmium::OSM::MultipolygonFromRelation::write_complex_poly_timer;
timer Osmium::OSM::MultipolygonFromRelation::assemble_ways_timer;
timer Osmium::OSM::MultipolygonFromRelation::assemble_nodes_timer;
timer Osmium::OSM::MultipolygonFromRelation::make_one_ring_timer;
timer Osmium::OSM::MultipolygonFromRelation::mor_polygonizer_timer;
timer Osmium::OSM::MultipolygonFromRelation::mor_union_timer;
timer Osmium::OSM::MultipolygonFromRelation::contains_timer;
timer Osmium::OSM::MultipolygonFromRelation::extra_polygons_timer;
timer Osmium::OSM::MultipolygonFromRelation::polygon_build_timer;
timer Osmium::OSM::MultipolygonFromRelation::inner_ring_touch_timer;
timer Osmium::OSM::MultipolygonFromRelation::polygon_intersection_timer;
timer Osmium::OSM::MultipolygonFromRelation::multipolygon_build_timer;
timer Osmium::OSM::MultipolygonFromRelation::multipolygon_write_timer;
timer Osmium::OSM::MultipolygonFromRelation::error_write_timer;
#endif // OSMIUM_WITH_MULTIPOLYGON_PROFILING

class SinglePass : public Osmium::Handler::Base {

    Osmium::Handler::NodeLocationStore *handler_node_location_store;
    Osmium::Handler::Javascript        *handler_javascript;

public:

    SinglePass(Osmium::Handler::NodeLocationStore *nls = NULL,
               Osmium::Handler::Javascript        *js  = NULL)
        : Base(),
          handler_node_location_store(nls),
          handler_javascript(js) {
    }

    void callback_init() {
        if (handler_node_location_store)
            handler_node_location_store->callback_init();
        handler_javascript->callback_init();
    }

    void callback_node(Osmium::OSM::Node *node) {
        if (handler_node_location_store)
            handler_node_location_store->callback_node(node);
        handler_javascript->callback_node(node);
    }

    void callback_way(Osmium::OSM::Way *way) {
        if (handler_node_location_store)
            handler_node_location_store->callback_way(way);
        handler_javascript->callback_way(way);
    }

    void callback_relation(Osmium::OSM::Relation *relation) {
        handler_javascript->callback_relation(relation);
    }

    void callback_final() {
        handler_javascript->callback_final();
    }

};

class DualPass1 : public Osmium::Handler::Base {

    Osmium::Handler::NodeLocationStore *handler_node_location_store;
    Osmium::Handler::Multipolygon      *handler_multipolygon;
    Osmium::Handler::Javascript        *handler_javascript;

public:

    DualPass1(Osmium::Handler::NodeLocationStore *nls = NULL,
              Osmium::Handler::Multipolygon      *mp  = NULL,
              Osmium::Handler::Javascript        *js  = NULL)
        : Base(),
          handler_node_location_store(nls),
          handler_multipolygon(mp),
          handler_javascript(js) {
    }

    void callback_init() {
        handler_multipolygon->callback_init();
        if (handler_node_location_store)
            handler_node_location_store->callback_init();
        handler_javascript->callback_init();
    }

    void callback_node(Osmium::OSM::Node *node) {
        handler_javascript->callback_node(node);
    }

    void callback_before_relations() {
        handler_multipolygon->callback_before_relations();
    }

    void callback_relation(Osmium::OSM::Relation *relation) {
        handler_multipolygon->callback_relation(relation);
        handler_javascript->callback_relation(relation);
    }

    void callback_after_relations() {
        handler_multipolygon->callback_after_relations();
        std::cerr << "1st pass finished" << std::endl;
    }

};

class DualPass2 : public Osmium::Handler::Base {

    Osmium::Handler::NodeLocationStore *handler_node_location_store;
    Osmium::Handler::Multipolygon      *handler_multipolygon;
    Osmium::Handler::Javascript        *handler_javascript;

public:

    DualPass2(Osmium::Handler::NodeLocationStore *nls = NULL,
              Osmium::Handler::Multipolygon      *mp  = NULL,
              Osmium::Handler::Javascript        *js  = NULL)
        : Base(),
          handler_node_location_store(nls),
          handler_multipolygon(mp),
          handler_javascript(js) {
    }

    void callback_node(Osmium::OSM::Node *node) {
        if (handler_node_location_store)
            handler_node_location_store->callback_node(node);
    }

    void callback_after_nodes() {
        if (handler_node_location_store)
            handler_node_location_store->callback_after_nodes();
    }

    void callback_way(Osmium::OSM::Way *way) {
        if (handler_node_location_store)
            handler_node_location_store->callback_way(way);
        handler_multipolygon->callback_way(way);
        handler_javascript->callback_way(way);
    }

    void callback_after_ways() {
        handler_multipolygon->callback_after_ways();
    }

    void callback_final() {
        handler_javascript->callback_final();
        handler_multipolygon->callback_final();
    }
};

/* ================================================== */

v8::Persistent<v8::Context> global_context;

void print_help() {
    std::cout << "osmjs [OPTIONS] OSMFILE [SCRIPT_ARG ...]" << std::endl \
              << "Options:" << std::endl \
              << "  --help, -h                       - This help message" << std::endl \
              << "  --debug, -d                      - Enable debugging output" << std::endl \
              << "  --include=FILE, -i FILE          - Include Javascript file (can be given several times)" << std::endl \
              << "  --javascript=FILE, -j FILE       - Process given Javascript file" << std::endl \
              << "  --location-store=STORE, -l STORE - Set location store (default: 'none')" << std::endl \
              << "  --no-repair, -r                  - Do not attempt to repair broken multipolygons" << std::endl \
              << "  --2pass, -2                      - Read OSMFILE twice and build multipolygons" << std::endl \
              << "Location stores:" << std::endl \
              << "  none        - Do not store node locations (you will have no way or polygon geometries)" << std::endl \
              << "  array       - Store node locations in large array (use for large OSM files)" << std::endl \
              << "  disk        - Store node locations on disk (use when low on memory)" << std::endl \
              << "  sparsetable - Store node locations in sparse table (use for small OSM files)" << std::endl;

}

std::string find_include_file(std::string filename) {
    std::vector<std::string> search_path;
    search_path.push_back(".");
    search_path.push_back("js");
    search_path.push_back(std::string(getenv("HOME")) + "/.osmjs");
    search_path.push_back("/usr/local/share/osmjs");
    search_path.push_back("/usr/share/osmjs");

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

Osmium::Handler::Javascript *handler_javascript;

void cbmp(Osmium::OSM::Multipolygon *multipolygon) {
    handler_javascript->callback_multipolygon(multipolygon);
}

int main(int argc, char *argv[]) {
    bool two_passes = false;
    bool attempt_repair = true;
    std::string javascript_filename;
    std::string osm_filename;
    std::vector<std::string> include_files;
    enum location_store_t {
        NONE,
        ARRAY,
        DISK,
        SPARSETABLE
    } location_store = NONE;

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

    bool debug = false;

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
                javascript_filename = optarg;
                break;
            case 'h':
                print_help();
                exit(0);
            case 'l':
                if (!strcmp(optarg, "none")) {
                    location_store = NONE;
                } else if (!strcmp(optarg, "array")) {
                    location_store = ARRAY;
                } else if (!strcmp(optarg, "disk")) {
                    location_store = DISK;
                } else if (!strcmp(optarg, "sparsetable")) {
                    location_store = SPARSETABLE;
                } else {
                    std::cerr << "Unknown location store: " << optarg << " (available are: 'none, 'array', 'disk' and 'sparsetable')" << std::endl;
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

    if (javascript_filename.empty()) {
        std::cerr << "No --javascript/-j option given" << std::endl;
        exit(1);
    }

    if (optind >= argc) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] OSMFILE [SCRIPT_ARG ...]" << std::endl;
        exit(1);
    } else {
        osm_filename = argv[optind];
    }

    if (two_passes && osm_filename == "-") {
        std::cerr << "Can't read from stdin when in dual-pass mode" << std::endl;
        exit(1);
    }

    Osmium::Framework osmium(debug);
    Osmium::OSMFile infile(osm_filename);

    v8::HandleScope handle_scope;

    v8::Handle<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New();
    global_template->Set(v8::String::New("print"), v8::FunctionTemplate::New(Osmium::Handler::Javascript::Print));
    global_template->Set(v8::String::New("include"), v8::FunctionTemplate::New(Osmium::Handler::Javascript::Include));

    global_context = v8::Persistent<v8::Context>::New(v8::Context::New(0, global_template));
    v8::Context::Scope context_scope(global_context);

    // put rest of the arguments into Javascript argv array
    v8::Handle<v8::Array> js_argv = v8::Array::New(argc-optind-1);
    for (int i=optind+1; i<argc; ++i) {
        v8::Local<v8::Integer> ii = v8::Integer::New(i-(optind+1));
        v8::Local<v8::String> s = v8::String::New(argv[i]);
        js_argv->Set(ii, s);
    }
    global_context->Global()->Set(v8::String::New("argv"), js_argv);

    Osmium::Javascript::Template::init();

    Osmium::Handler::NodeLocationStore *handler_node_location_store;
    if (location_store == ARRAY) {
        handler_node_location_store = new Osmium::Handler::NLS_Array();
    } else if (location_store == DISK) {
        handler_node_location_store = new Osmium::Handler::NLS_Disk();
    } else if (location_store == SPARSETABLE) {
        handler_node_location_store = new Osmium::Handler::NLS_Sparsetable();
    } else {
        handler_node_location_store = NULL;
    }
    handler_javascript = new Osmium::Handler::Javascript(include_files, javascript_filename.c_str());

    if (two_passes) {
        Osmium::Handler::Multipolygon *handler_multipolygon = new Osmium::Handler::Multipolygon(attempt_repair, cbmp);
        infile.read<DualPass1>(new DualPass1(handler_node_location_store, handler_multipolygon, handler_javascript));
        infile.read<DualPass2>(new DualPass2(handler_node_location_store, handler_multipolygon, handler_javascript));
        delete handler_multipolygon;
    } else {
        infile.read<SinglePass>(new SinglePass(handler_node_location_store, handler_javascript));
    }
    delete handler_javascript;
    delete handler_node_location_store;

    global_context.Dispose();
}

