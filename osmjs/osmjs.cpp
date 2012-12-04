/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <cstdlib>
#include <getopt.h>
#include <unistd.h>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT
#include <osmium.hpp>

#include <osmium/javascript.hpp>
#include <osmium/storage/byid/fixed_array.hpp>
#include <osmium/storage/byid/sparse_table.hpp>
#include <osmium/storage/byid/mmap_file.hpp>
#include <osmium/storage/byid/vector.hpp>
#ifdef __linux__
#  include <osmium/storage/byid/mmap_anon.hpp>
#endif
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/multipolygon/assembler.hpp>

typedef Osmium::Storage::ById::Base<Osmium::OSM::Position> storage_byid_t;
typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;
typedef Osmium::Handler::CoordinatesForWays<storage_byid_t, storage_mmap_t> cfw_handler_t;

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
              << "  vector      - Store node locations in vector of ID/Value pairs (very low memory overhead for small OSM datasets)\n"
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

int main(int argc, char* argv[]) {
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
        SPARSETABLE,
        VECTOR
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
                } else if (!strcmp(optarg, "vector")) {
                    location_store = VECTOR;
                } else if (!strcmp(optarg, "sparsetable")) {
                    location_store = SPARSETABLE;
                } else {
                    std::cerr << "Unknown location store: " << optarg << " (available are: 'none, 'array', 'disk', 'vector' and 'sparsetable')" << std::endl;
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

    if (multipolygon && location_store == NONE) {
        std::cerr << "You need to set the location store with -l, otherwise multipolygon assembly will not work.\n";
        exit(1);
    }

    Osmium::OSMFile infile(osm_filename);

    v8::HandleScope handle_scope;

    v8::Handle<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New();
    global_template->Set(v8::String::New("print"), v8::FunctionTemplate::New(Osmium::Javascript::Handler::Print));
    global_template->Set(v8::String::New("include"), v8::FunctionTemplate::New(Osmium::Javascript::Handler::Include));

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
    if (location_store == DISK) {
        store_pos = new Osmium::Storage::ById::MmapFile<Osmium::OSM::Position>();
    } else if (location_store == ARRAY) {
#ifdef __linux__
        store_pos = new Osmium::Storage::ById::MmapAnon<Osmium::OSM::Position>();
#else
        std::cerr << "Option -l array is not available on non-linux system. Use -l disk instead.\n";
        exit(1);
#endif
    } else if (location_store == SPARSETABLE) {
        store_pos = new Osmium::Storage::ById::SparseTable<Osmium::OSM::Position>();
    } else if (location_store == VECTOR) {
    	store_pos = new Osmium::Storage::ById::Vector<Osmium::OSM::Position>();
    }
    Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> store_neg;
    Osmium::Javascript::Handler handler_javascript(include_files, javascript_filename.c_str());
    handler_javascript.set_debug_level(debug ? 1 : 0);

    if (two_passes) {
        typedef Osmium::MultiPolygon::Assembler<Osmium::Javascript::Handler> assembler_t;
        assembler_t assembler(handler_javascript, attempt_repair);

        cfw_handler_t handler_cfw(*store_pos, store_neg);

        typedef Osmium::Handler::Sequence<cfw_handler_t, assembler_t::HandlerPass2> sequence_handler_t;
        sequence_handler_t sequence_handler(handler_cfw, assembler.handler_pass2());

        Osmium::Input::read(infile, assembler.handler_pass1());
        Osmium::Input::read(infile, sequence_handler);
    } else if (store_pos) {
        cfw_handler_t handler_cfw(*store_pos, store_neg);

        typedef Osmium::Handler::Sequence<cfw_handler_t, Osmium::Javascript::Handler> sequence_handler_t;
        sequence_handler_t sequence_handler(handler_cfw, handler_javascript);

        Osmium::Input::read(infile, sequence_handler);
    } else {
        Osmium::Input::read(infile, handler_javascript);
    }

    delete store_pos;

    global_context.Dispose();

    google::protobuf::ShutdownProtobufLibrary();
}

