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
        std::cerr << "Unknown index type '" << index_type << "'. (Allowed are 'n', 'w', and 'r'.)\n";
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
            std::cerr << "No object of type '" << index_type << "' with id " << id << " found.\n";
            exit(1);
        }
    } else if (!map_file.empty()) {
        int map_fd = ::open(map_file.c_str(), O_RDONLY);
        if (map_fd < 0) {
            std::cerr << "Can't open map file '" << map_file << "': " << strerror(errno) << "\n";
            exit(2);
        }

        Osmium::Storage::Member::Mmap map(map_fd);

        std::pair<Osmium::Storage::Member::Mmap::id_id_t*, Osmium::Storage::Member::Mmap::id_id_t*> result = map.get(id);
        for (Osmium::Storage::Member::Mmap::id_id_t* it = result.first; it != result.second; ++it) {
            std::cout << it->second << "\n";
        }
    }
}

