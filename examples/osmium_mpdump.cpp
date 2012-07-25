/*

  Create multipolygons and dump them to stdout.

*/

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

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
        Osmium::Geometry::MultiPolygon mp(*area);
        std::cout << "Area " << (area->nodes().size() == 0 ? "from relation" : "from way")
                  << " id=" << area->id()
                  << " version=" << area->version()
                  << " timestamp=" << area->timestamp()
                  << " uid=" << area->uid()
                  << " user=" << area->user()
                  << "\n  " << mp.as_WKT() << "\n";

        BOOST_FOREACH(const Osmium::OSM::Tag& tag, area->tags()) {
            std::cout << "  " << tag.key() << "=" << tag.value() << "\n";
        }

        std::cout << "\n";
    }

};

/* ================================================== */

typedef Osmium::Storage::ById::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;

int main(int argc, char *argv[]) {
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
    assembler.debug_level(1);

    typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;
    cfw_handler_t cfw_handler(store_pos, store_neg);

    typedef Osmium::Handler::Sequence<cfw_handler_t, assembler_t::HandlerPass2> sequence_handler_t;
    sequence_handler_t sequence_handler(cfw_handler, assembler.handler_pass2());

    std::cerr << "First pass...\n";
    Osmium::Input::read(infile, assembler.handler_pass1());

    std::cerr << "Second pass...\n";
    Osmium::Input::read(infile, sequence_handler);
}

