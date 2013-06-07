/*

  This is a small tool to get an object from a dump and index file.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/index.hpp>
#include <osmium/ser/deserializer.hpp>

void full() {
    std::cerr << "full\n";
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " DUMPFILE INDEXFILE ID\n";
        exit(1);
    }

    const char* dumpfile = argv[1];
    const char* indexfile = argv[2];
    osm_object_id_t id = atol(argv[3]);

    int fd = ::open(dumpfile, O_RDONLY);
    if (fd < 0) {
        std::cerr << "Can't read file " << dumpfile << "\n";
        exit(1);
    }

    struct stat file_stat;
    if (::fstat(fd, &file_stat) < 0) {
        std::cerr << "Can't stat file " << dumpfile << "\n";
        exit(1);
    }
    
    size_t bufsize = file_stat.st_size;
    char* mem = reinterpret_cast<char*>(::mmap(NULL, bufsize, PROT_READ, MAP_SHARED, fd, 0));
    if (!mem) {
        std::cerr << "Can't mmap file " << dumpfile << "\n";
        exit(1);
    }

    int index_fd = ::open(indexfile, O_RDONLY);
    if (index_fd < 0) {
        std::cerr << "Can't read file " << indexfile << "\n";
        exit(1);
    }

    Osmium::Ser::Index::MemWithId index(index_fd);
    size_t pos = index.get(id);
    size_t length = reinterpret_cast<Osmium::Ser::TypedItem*>(&mem[pos])->size();
    std::cout << "length: " << length << "\n";
    Osmium::Ser::Buffer buffer(mem + pos, length, length, boost::bind(&full));

    Osmium::Handler::Debug debug;
    Osmium::Ser::Deserializer<Osmium::Handler::Debug> deser(buffer, debug);
    deser.feed();
//    deser.dump();
}

