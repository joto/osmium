/*

  This is a small tool to get an object from a dump and index file.

  The code in this example file is released into the Public Domain.

*/

#include <cerrno>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <string>

#include <osmium.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/index.hpp>
#include <osmium/storage/member/mmap.hpp>
#include <osmium/ser/debug.hpp>

typedef Osmium::Storage::Member::Mmap map_t;

void print_help() {
    std::cout << "osmium_serget [OPTIONS] DIR TYPE ID\n" \
              << "Output object of type TYPE with ID from data files/indexes in DIR.\n" \
              << "TYPE is one of 'n', 'w', and 'r'.\n" \
              << "\nOptions:\n" \
              << "  -h, --help       This help message\n" \
              << "  -s, --with-size  Report sizes of objects\n";
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    static struct option long_options[] = {
        {"help",      no_argument, 0, 'h'},
        {"with-size", no_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    bool with_size = false;

    while (true) {
        int c = getopt_long(argc, argv, "hs", long_options, 0);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'h':
                print_help();
                exit(0);
            case 's':
                with_size = true;
                break;
            default:
                exit(2);
        }
    }

    int remaining_args = argc - optind;
    if (remaining_args != 3) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] DIR TYPE ID\n";
        exit(2);
    }

    std::string dir(argv[optind]);
    std::string data_file(dir + "/data.osm.ser");

    std::string index_type(argv[optind+1]);

    std::string index_file;
    std::string map_file;
    if (index_type == "n") {
        index_file = dir + "/nodes.idx";
    } else if (index_type == "w") {
        index_file = dir + "/ways.idx";
    } else if (index_type == "r") {
        index_file = dir + "/relations.idx";
    } else if (index_type == "n2w") {
        map_file = dir + "/node2way.map";
    } else if (index_type == "n2r") {
        map_file = dir + "/node2rel.map";
    } else if (index_type == "w2r") {
        map_file = dir + "/way2rel.map";
    } else if (index_type == "r2r") {
        map_file = dir + "/rel2rel.map";
    } else {
        std::cerr << "Unknown index type '" << index_type << "'. (Allowed are 'n', 'w', 'r', 'n2w', 'n2r', 'w2r', and 'r2r'.)\n";
        exit(2);
    }

    osm_object_id_t id = atoll(argv[optind+2]);

    if (!index_file.empty()) {
        int index_fd = ::open(index_file.c_str(), O_RDONLY);
        if (index_fd < 0) {
            std::cerr << "Can't open index file '" << index_file << "': " << strerror(errno) << "\n";
            exit(2);
        }

        Osmium::Ser::Index::MemMapWithId index(index_fd);
        try {
            size_t pos = index.get(id);

            typedef Osmium::Ser::BufferManager::FileInput manager_t;
            manager_t manager(data_file);

            Osmium::Ser::Dump dump(std::cout, with_size);
            dump(manager.get<Osmium::Ser::TypedItem>(pos));
        } catch (Osmium::Ser::Index::NotFound&) {
            exit(1);
        }
    } else if (!map_file.empty()) {
        int map_fd = ::open(map_file.c_str(), O_RDONLY);
        if (map_fd < 0) {
            std::cerr << "Can't open map file '" << map_file << "': " << strerror(errno) << "\n";
            exit(2);
        }

        map_t map(map_fd);

        std::pair<map_t::iterator, map_t::iterator> result = map.get(id);
        if (result.first == result.second) {
            exit(1);
        }

        for (map_t::iterator it = result.first; it != result.second; ++it) {
            std::cout << it->second << "\n";
        }
    }
}

