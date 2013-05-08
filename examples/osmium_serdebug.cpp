/*

  This is a small tool to show the contents of the input file
  that contains serialized format on stdout.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/ser/buffer.hpp>
#include <osmium/ser/item.hpp>

void full() {
    std::cerr << "full\n";
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " FILE\n";
        exit(1);
    }

    std::cerr << "opening file " << argv[1] << "\n";
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        std::cerr << "Can't read file " << argv[1] << "\n";
        exit(1);
    }

    size_t bufsize = 10000;
    char* mem = reinterpret_cast<char*>(mmap(NULL, bufsize, PROT_READ, MAP_SHARED, fd, 0));
    if (!mem) {
        std::cerr << "Can't mmap file " << argv[1] << "\n";
        exit(1);
    }

    Osmium::Ser::Buffer buffer(mem, bufsize, boost::bind(&full));

    Osmium::Ser::Deserializer<Osmium::Handler::Debug> deser(buffer);
    Osmium::Handler::Debug debug;
    deser.feed(debug);
}

