
#include <cstdlib>
#include <getopt.h>
#include <unistd.h>

#include <osmium.hpp>
#include <osmium/storage/byid.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/handler/multipolygon.hpp>

#ifdef OSMIUM_WITH_MULTIPOLYGON_PROFILING
std::vector<std::pair<std::string, timer *> > Osmium::OSM::AreaFromRelation::timers;

timer Osmium::OSM::AreaFromRelation::write_complex_poly_timer;
timer Osmium::OSM::AreaFromRelation::assemble_ways_timer;
timer Osmium::OSM::AreaFromRelation::assemble_nodes_timer;
timer Osmium::OSM::AreaFromRelation::make_one_ring_timer;
timer Osmium::OSM::AreaFromRelation::mor_polygonizer_timer;
timer Osmium::OSM::AreaFromRelation::mor_union_timer;
timer Osmium::OSM::AreaFromRelation::contains_timer;
timer Osmium::OSM::AreaFromRelation::extra_polygons_timer;
timer Osmium::OSM::AreaFromRelation::polygon_build_timer;
timer Osmium::OSM::AreaFromRelation::inner_ring_touch_timer;
timer Osmium::OSM::AreaFromRelation::polygon_intersection_timer;
timer Osmium::OSM::AreaFromRelation::multipolygon_build_timer;
timer Osmium::OSM::AreaFromRelation::multipolygon_write_timer;
timer Osmium::OSM::AreaFromRelation::error_write_timer;
#endif // OSMIUM_WITH_MULTIPOLYGON_PROFILING

typedef Osmium::Storage::ById<Osmium::OSM::Position> storage_byid_t;
typedef Osmium::Storage::Mmap<Osmium::OSM::Position> storage_mmap_t;
typedef Osmium::Handler::CoordinatesForWays<storage_byid_t, storage_mmap_t> cfw_handler_t;

class SinglePass : public Osmium::Handler::Base {

    cfw_handler_t* handler_cfw;
    Osmium::Handler::Javascript* handler_javascript;

public:

    SinglePass(cfw_handler_t* cfw = NULL,
               Osmium::Handler::Javascript* js  = NULL)
        : Base(),
          handler_cfw(cfw),
          handler_javascript(js) {
    }

    void init(Osmium::OSM::Meta& meta) {
        if (handler_cfw) {
            handler_cfw->init(meta);
        }
        handler_javascript->init(meta);
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& node) {
        if (handler_cfw) {
            handler_cfw->node(node);
        }
        handler_javascript->node(node);
    }

    void after_nodes() {
        if (handler_cfw)
            handler_cfw->after_nodes();
    }

    void way(const shared_ptr<Osmium::OSM::Way>& way) {
        if (handler_cfw) {
            handler_cfw->way(way);
        }
        handler_javascript->way(way);
    }

    void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
        handler_javascript->relation(relation);
    }

    void final() {
        handler_javascript->final();
    }

};

class DualPass1 : public Osmium::Handler::Base {

    cfw_handler_t* handler_cfw;
    Osmium::Handler::Multipolygon* handler_multipolygon;
    Osmium::Handler::Javascript* handler_javascript;

public:

    DualPass1(cfw_handler_t* cfw = NULL,
              Osmium::Handler::Multipolygon* mp  = NULL,
              Osmium::Handler::Javascript* js  = NULL)
        : Base(),
          handler_cfw(cfw),
          handler_multipolygon(mp),
          handler_javascript(js) {
    }

    void init(Osmium::OSM::Meta& meta) {
        if (handler_multipolygon) {
            handler_multipolygon->init(meta);
        }
        if (handler_cfw) {
            handler_cfw->init(meta);
        }
        handler_javascript->init(meta);
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& node) {
        handler_javascript->node(node);
    }

    void before_relations() {
        if (handler_multipolygon) {
            handler_multipolygon->before_relations();
        }
    }

    void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
        if (handler_multipolygon) {
            handler_multipolygon->relation(relation);
        }
        handler_javascript->relation(relation);
    }

    void after_relations() {
        if (handler_multipolygon) {
            handler_multipolygon->after_relations();
        }
        std::cerr << "1st pass finished" << std::endl;
    }

};

class DualPass2 : public Osmium::Handler::Base {

    cfw_handler_t* handler_cfw;
    Osmium::Handler::Multipolygon* handler_multipolygon;
    Osmium::Handler::Javascript* handler_javascript;

public:

    DualPass2(cfw_handler_t* cfw = NULL,
              Osmium::Handler::Multipolygon* mp  = NULL,
              Osmium::Handler::Javascript* js  = NULL)
        : Base(),
          handler_cfw(cfw),
          handler_multipolygon(mp),
          handler_javascript(js) {
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& node) {
        if (handler_cfw) {
            handler_cfw->node(node);
        }
    }

    void after_nodes() {
        if (handler_cfw) {
            handler_cfw->after_nodes();
        }
    }

    void way(const shared_ptr<Osmium::OSM::Way>& way) {
        if (handler_cfw) {
            handler_cfw->way(way);
        }
        if (handler_multipolygon) {
            handler_multipolygon->way(way);
        }
        handler_javascript->way(way);
    }

    void after_ways() {
        if (handler_multipolygon) {
            handler_multipolygon->after_ways();
        }
    }

    void final() {
        handler_javascript->final();
        if (handler_multipolygon) {
            handler_multipolygon->final();
        }
    }
};

/* ================================================== */

v8::Persistent<v8::Context> global_context;

void print_help() {
    std::cout << "osmjs [OPTIONS] OSMFILE [SCRIPT_ARG ...]\n"
              << "Options:\n"
              << "  --help, -h                       - This help message\n"
              << "  --debug, -d                      - Enable debugging output\n"
              << "  --include=FILE, -i FILE          - Include Javascript file (can be given several times)\n"
              << "  --javascript=FILE, -j FILE       - Process given Javascript file\n"
              << "  --location-store=STORE, -l STORE - Set location store (default: 'none')\n"
              << "  --no-repair, -r                  - Do not attempt to repair broken multipolygons\n"
              << "  --2pass, -2                      - Read OSMFILE twice\n"
              << "  --multipolygon, -m               - Build multipolygons (implies -2)\n"
              << "Location stores:\n"
              << "  none        - Do not store node locations (you will have no way or polygon geometries)\n"
              << "  array       - Store node locations in large array (use for large OSM files)\n"
              << "  disk        - Store node locations on disk (use when low on memory)\n"
              << "  sparsetable - Store node locations in sparse table (use for small OSM files)\n"
              ;
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

Osmium::Handler::Javascript* handler_javascript;

void cbmp(Osmium::OSM::Area* area) {
    handler_javascript->area(area);
}

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);

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
        {"multipolygon",         no_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    bool debug = false;
    bool multipolygon = false;

    while (1) {
        int c = getopt_long(argc, argv, "dhi:j:l:r2m", long_options, 0);
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
            case 'm':
                multipolygon = true;
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

    if (two_passes && ! multipolygon) {
        std::cerr << "Warning! The command line option -2 has changed its meaning.\nIt now only enables the two-pass mode, multipolygon assembly has to be enabled with -m.\n";
    }

    Osmium::init(debug);
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

    storage_byid_t* store_pos = NULL;
    if (location_store == ARRAY) {
        store_pos = new Osmium::Storage::Mmap<Osmium::OSM::Position>();
    } else if (location_store == DISK) {
        std::string filename("");
        store_pos = new Osmium::Storage::Mmap<Osmium::OSM::Position>(filename);
    } else if (location_store == SPARSETABLE) {
        store_pos = new Osmium::Storage::SparseTable<Osmium::OSM::Position>();
    }
    Osmium::Storage::Mmap<Osmium::OSM::Position> store_neg;
    cfw_handler_t* handler_cfw = (store_pos == NULL) ? NULL : new cfw_handler_t(*store_pos, store_neg);
    handler_javascript = new Osmium::Handler::Javascript(include_files, javascript_filename.c_str());

    if (two_passes) {
        Osmium::Handler::Multipolygon* handler_multipolygon = NULL;
        if (multipolygon) {
            handler_multipolygon = new Osmium::Handler::Multipolygon(attempt_repair, cbmp);
        }
        DualPass1 handler1(handler_cfw, handler_multipolygon, handler_javascript);
        infile.read(handler1);
        DualPass2 handler2(handler_cfw, handler_multipolygon, handler_javascript);
        infile.read(handler2);
        delete handler_multipolygon;
    } else {
        SinglePass handler(handler_cfw, handler_javascript);
        infile.read(handler);
    }

    delete handler_javascript;
    delete handler_cfw;
    delete store_pos;

    global_context.Dispose();
}

