/*

  This is a small tool to dump the contents of the input file.

*/

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <cstdlib>

#include <osmium.hpp>
#include <osmium/handler/debug.hpp>

class MyDebugHandler : public Osmium::Handler::Base {

    Osmium::Handler::Debug handler_debug;

public:

    void init() {
        std::cout << "init" << std::endl;
    }

    void before_nodes() {
        std::cout << "before_nodes" << std::endl;
    }

    void node(Osmium::OSM::Node *node) {
        handler_debug.node(node);
    }

    void after_nodes() {
        std::cout << "after_nodes" << std::endl;
    }

    void before_ways() {
        std::cout << "before_ways" << std::endl;
    }

    void way(Osmium::OSM::Way *way) {
        handler_debug.way(way);
    }

    void after_ways() {
        std::cout << "after_ways" << std::endl;
    }

    void before_relations() {
        std::cout << "before_relations" << std::endl;
    }

    void relation(Osmium::OSM::Relation *relation) {
        handler_debug.relation(relation);
    }

    void after_relations() {
        std::cout << "after_relations" << std::endl;
    }

    void final() {
        std::cout << "final" << std::endl;
    }
};

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::init(true);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    MyDebugHandler handler;
    infile.read(handler);
}

