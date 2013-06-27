/*

  This is a small tool to show the contents of an input file
  containing serialized OSM format.

  The code in this example file is released into the Public Domain.

*/

#include <getopt.h>
#include <iostream>
#include <string>

#include <osmium.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/debug.hpp>

void print_help() {
    std::cout << "osmium_serdebug [OPTIONS] [DIR]\n" \
              << "Print content of data file in DIR to stdout.\n" \
              << "\nIf no DIR is given, current dir is assumed.\n" \
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

    std::string dir(".");

    if (remaining_args == 0) {
        // nothing
    } else if (remaining_args == 1) {
        dir = argv[optind];
    } else {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] [DIR]\n";
        exit(2);
    }

    std::string data_file(dir + "/data.osm.ser");

    typedef Osmium::Ser::BufferManager::FileInput manager_t;
    manager_t manager(data_file);

    Osmium::Ser::Dump dump(std::cout, with_size);
    std::for_each(manager.begin(), manager.end(), dump);
}

