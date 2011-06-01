/*

  This is a small tool to time osmium.

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
#include <time.h>
#include <sys/times.h>

#define OSMIUM_MAIN
#include <osmium.hpp>

class MyTimerHandler : public Osmium::Handler::Base {

    uint64_t nodes;
    uint64_t ways;
    uint64_t relations;

public:

    MyTimerHandler() : nodes(0), ways(0), relations(0) {
    }

    void callback_node(Osmium::OSM::Node * /*node*/) {
        nodes++;
    }

    void callback_way(Osmium::OSM::Way * /*way*/) {
        ways++;
    }

    void callback_relation(Osmium::OSM::Relation * /*relation*/) {
        relations++;
    }

    void callback_final() {
        std::cout << "nodes: " << nodes << "  ways: " << ways << "  relations: " << relations << std::endl;
    }
};

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::Framework osmium(true);

    time_t t0 = time(NULL);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    osmium.parse_osmfile<MyTimerHandler>(argv[1]);

    struct tms tms;
    times(&tms);
    std::cout << "user time: " << ((double)tms.tms_utime) / sysconf(_SC_CLK_TCK) << "s   system time: " << ((double)tms.tms_stime) / sysconf(_SC_CLK_TCK) << "s  wallclock time: " << time(NULL) - t0 << "s" << std::endl;
}

