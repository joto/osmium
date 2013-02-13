/*

  Create multipolygons and dump them to stdout.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/storage/byid/sparse_table.hpp>
#include <osmium/storage/byid/mmap_file.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/multipolygon/assembler.hpp>
#include <osmium/geometry/multipolygon.hpp>

/* ================================================== */

class DumpHandler : public Osmium::Handler::Base {

public:

    DumpHandler() : Osmium::Handler::Base() {
    }

    void area(const shared_ptr<Osmium::OSM::Area const>& area) {
        Osmium::Geometry::MultiPolygon multipolygon(*area);

        std::cout << "Area " << (area->from_way() ? "from way" : "from relation")
                  << " id=" << area->id() << " (orig_id=" << area->orig_id() << ")"
                  << " version=" << area->version()
                  << " timestamp=" << area->timestamp()
                  << " uid=" << area->uid()
                  << " user=" << area->user()
                  << "\n  " << multipolygon.as_WKT() << "\n";

        BOOST_FOREACH(const Osmium::OSM::Tag& tag, area->tags()) {
            std::cout << "  " << tag.key() << "=" << tag.value() << "\n";
        }

        std::cout << "\n";
    }

};

/* ================================================== */

typedef Osmium::Storage::ById::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);

    bool attempt_repair = true;

    storage_sparsetable_t store_pos;
    storage_mmap_t store_neg;

    DumpHandler dump_handler;

    typedef Osmium::MultiPolygon::Assembler<DumpHandler> assembler_t;
    assembler_t assembler(dump_handler, attempt_repair);
    assembler.set_debug_level(1);

    typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;
    cfw_handler_t cfw_handler(store_pos, store_neg);

    typedef Osmium::Handler::Sequence<cfw_handler_t, assembler_t::HandlerPass2> sequence_handler_t;
    sequence_handler_t sequence_handler(cfw_handler, assembler.handler_pass2());

    std::cerr << "First pass...\n";
    Osmium::Input::read(infile, assembler.handler_pass1());

    std::cerr << "Second pass...\n";
    Osmium::Input::read(infile, sequence_handler);

    google::protobuf::ShutdownProtobufLibrary();
}

