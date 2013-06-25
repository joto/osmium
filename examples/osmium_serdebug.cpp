/*

  This is a small tool to show the contents of the input file
  that contains serialized format on stdout.

  The code in this example file is released into the Public Domain.

*/

#include <getopt.h>
#include <iostream>

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/deserializer.hpp>

void print_help() {
    std::cout << "osmium_serdebug [OPTIONS] INFILE\n" \
              << "\nOptions:\n" \
              << "  -h, --help     This help message\n" \
              << "  -d, --dump     Simple dump output format\n";
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"dump", no_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    bool dump = false;

    while (true) {
        int c = getopt_long(argc, argv, "hd", long_options, 0);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'd':
                dump = true;
                break;
            case 'h':
                print_help();
                exit(0);
            default:
                exit(1);
        }
    }

    int remaining_args = argc - optind;
    if (remaining_args != 1) {
        std::cerr << "Usage: " << argv[0] << " [--dump] FILE\n";
        exit(1);
    }

    std::string infile(argv[optind]);

    typedef Osmium::Ser::BufferManager::FileInput manager_t;
    manager_t manager(infile);

    Osmium::Handler::Debug debug;
    Osmium::Ser::Deserializer<manager_t, Osmium::Handler::Debug> deser(manager, debug);

    if (dump) {
        deser.dump();
    } else {
        deser.feed();
    }
}

