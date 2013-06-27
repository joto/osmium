/*

  This is a small tool to get an object from a dump and index file.

  The code in this example file is released into the Public Domain.

*/

#include <getopt.h>
#include <iostream>
#include <string>

#include <osmium.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/index.hpp>
#include <osmium/ser/debug.hpp>

void print_help() {
    std::cout << "osmium_serget [OPTIONS] DUMPFILE INDEXFILE ID\n" \
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
                exit(1);
        }
    }

    int remaining_args = argc - optind;
    if (remaining_args != 3) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] DUMPFILE INDEXFILE ID\n";
        exit(1);
    }

    std::string dumpfile(argv[optind]);
    const char* indexfile = argv[optind+1];
    osm_object_id_t id = atol(argv[optind+2]);

    int index_fd = ::open(indexfile, O_RDONLY);
    if (index_fd < 0) {
        std::cerr << "Can't read index file " << indexfile << "\n";
        exit(1);
    }

    Osmium::Ser::Index::MemMapWithId index(index_fd);
    size_t pos = index.get(id);

    typedef Osmium::Ser::BufferManager::FileInput manager_t;
    manager_t manager(dumpfile);

    Osmium::Ser::Dump dump(std::cout, with_size);
    dump(manager.get<Osmium::Ser::TypedItem>(pos));
}

