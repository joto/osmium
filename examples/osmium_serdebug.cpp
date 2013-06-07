/*

  This is a small tool to show the contents of the input file
  that contains serialized format on stdout.

  The code in this example file is released into the Public Domain.

*/

#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/deserializer.hpp>

void full() {
    std::cerr << "full\n";
}

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

    const char *infile = argv[optind];
    int fd = open(infile, O_RDONLY);
    if (fd < 0) {
        std::cerr << "Can't read file " << infile << "\n";
        exit(1);
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        std::cerr << "Can't stat file " << infile << "\n";
        exit(1);
    }
    
    size_t bufsize = file_stat.st_size;
    char* mem = reinterpret_cast<char*>(mmap(NULL, bufsize, PROT_READ, MAP_SHARED, fd, 0));
    if (!mem) {
        std::cerr << "Can't mmap file " << infile << "\n";
        exit(1);
    }

    Osmium::Ser::Buffer buffer(mem, bufsize, bufsize, boost::bind(&full));

    Osmium::Handler::Debug debug;
    Osmium::Ser::Deserializer<Osmium::Handler::Debug> deser(buffer, debug);

    if (dump) {
        deser.dump();
    } else {
        deser.feed();
    }
}

