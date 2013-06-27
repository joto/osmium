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
#include <osmium/ser/update_handler.hpp>
#include <osmium/ser/handler.hpp>
#include <osmium/ser/index.hpp>
#include <osmium/storage/member/multimap.hpp>

typedef Osmium::Ser::Index::VectorWithId index_t;
typedef Osmium::Storage::Member::MultiMap map_t;

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    assert(sizeof(Osmium::Ser::TypedItem) % 8 == 0);
    assert(sizeof(Osmium::Ser::Object)    % 8 == 0);
    assert(sizeof(Osmium::Ser::Node)      % 8 == 0);
    assert(sizeof(Osmium::Ser::Way)       % 8 == 0);
    assert(sizeof(Osmium::Ser::Relation)  % 8 == 0);

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE DUMPFILE\n";
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);

    std::string dumpfile(argv[2]);
    int dump_fd = ::open(dumpfile.c_str(), O_WRONLY | O_CREAT, 0666);
    if (dump_fd < 0) {
        throw std::runtime_error(std::string("Can't open dump file ") + dumpfile);
    }

    typedef Osmium::Ser::BufferManager::File manager_t;
    manager_t manager(dumpfile, 10 * 1024 * 1024);

    index_t node_index;
    index_t way_index;
    index_t relation_index;

    map_t map_way2node;
    map_t map_node2relation;
    map_t map_way2relation;
    map_t map_relation2relation;

    typedef Osmium::Ser::UpdateHandler::ObjectsWithDeps<map_t, map_t, map_t, map_t> update_handler_t;
    update_handler_t update_handler(map_way2node, map_node2relation, map_way2relation, map_relation2relation);

    Osmium::Ser::Handler<manager_t, update_handler_t, index_t, index_t, index_t>
        handler(manager, update_handler, false, node_index, way_index, relation_index);

    Osmium::Input::read(infile, handler);

    int fd = ::open("nodes.idx", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open nodes.idx");
    }
    node_index.dump(fd);
    close(fd);

    fd = ::open("ways.idx", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open ways.idx");
    }
    way_index.dump(fd);
    close(fd);

    fd = ::open("relations.idx", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        throw std::runtime_error("Can't open relations.idx");
    }
    relation_index.dump(fd);
    close(fd);

    map_way2node.dump("n-w ");
    map_node2relation.dump("n-r ");
    map_way2relation.dump("w-r ");
    map_relation2relation.dump("r-r ");

    google::protobuf::ShutdownProtobufLibrary();
}

