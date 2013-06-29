/*

  This is a small tool to show the contents of an input file
  containing serialized OSM format.

  The code in this example file is released into the Public Domain.

*/

#include <getopt.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <osmium.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/debug.hpp>

void print_help() {
    std::cout << "osmium_serdebug [OPTIONS] DIR|FILE\n" \
              << "Print content of data file FILE (or 'data.osm.ser' in DIR) to stdout.\n" \
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

    if (remaining_args != 1) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] DIR|FILE\n";
        exit(2);
    }

    std::string dir(argv[optind]);

    struct stat file_stat;
    if (::stat(dir.c_str(), &file_stat) < 0) {
        std::cerr << "Can't stat '" << dir << "': " << strerror(errno) << "\n";
        exit(2);
    }
    
    std::string data_file(dir);
    if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
        data_file += "/data.osm.ser";
    }

    typedef Osmium::Ser::BufferManager::FileInput manager_t;
    manager_t manager(data_file);

    Osmium::Ser::Dump dump(std::cout, with_size);
    std::for_each(manager.begin(), manager.end(), dump);
}

