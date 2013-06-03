/*

  This is a small tool to dump the contents of the input file
  in serialized format to stdout.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/handler.hpp>
#include <osmium/ser/index.hpp>

typedef Osmium::Ser::Index::VectorWithId index_t;

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    Osmium::Ser::BufferManager::Malloc manager(1024 * 1024);
    index_t node_index;
    index_t way_index;
    index_t relation_index;
    Osmium::Ser::Handler<Osmium::Ser::BufferManager::Malloc, index_t, index_t, index_t> handler(manager, node_index, way_index, relation_index);
    Osmium::Input::read(infile, handler);

    int fd = ::open("nodes.idx", O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open nodes.idx");
    }
    node_index.dump(fd);
    close(fd);

    fd = ::open("ways.idx", O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open ways.idx");
    }
    way_index.dump(fd);
    close(fd);

    fd = ::open("relations.idx", O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open relations.idx");
    }
    relation_index.dump(fd);
    close(fd);

    google::protobuf::ShutdownProtobufLibrary();
}

