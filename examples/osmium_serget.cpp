/*

  This is a small tool to get an object from a dump and index file.

  The code in this example file is released into the Public Domain.

*/

#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/index.hpp>
#include <osmium/ser/deserializer.hpp>

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " DUMPFILE INDEXFILE ID\n";
        exit(1);
    }

    std::string dumpfile(argv[1]);
    const char* indexfile = argv[2];
    osm_object_id_t id = atol(argv[3]);

    int index_fd = ::open(indexfile, O_RDONLY);
    if (index_fd < 0) {
        std::cerr << "Can't read file " << indexfile << "\n";
        exit(1);
    }

    Osmium::Ser::Index::MemMapWithId index(index_fd);
    size_t pos = index.get(id);

    typedef Osmium::Ser::BufferManager::FileInput manager_t;
    manager_t manager(dumpfile);

    Osmium::Handler::Debug debug;
    Osmium::Ser::Deserializer<manager_t, Osmium::Handler::Debug> deser(manager, debug);
    deser.parse_item(manager.get<Osmium::Ser::TypedItem>(pos));
}

