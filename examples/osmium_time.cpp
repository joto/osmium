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

#include <osmium.hpp>

class MyTimerHandler : public Osmium::Handler::Base {

    uint64_t m_nodes;
    uint64_t m_ways;
    uint64_t m_relations;

public:

    MyTimerHandler() : m_nodes(0), m_ways(0), m_relations(0) {
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& /*node*/) {
        m_nodes++;
    }

    void way(const shared_ptr<Osmium::OSM::Way const>& /*way*/) {
        m_ways++;
    }

    void relation(const shared_ptr<Osmium::OSM::Relation const>& /*relation*/) {
        m_relations++;
    }

    void final() {
        std::cout << "nodes: " << m_nodes << "  ways: " << m_ways << "  relations: " << m_relations << std::endl;
    }
};

/* ================================================== */

int main(int argc, char *argv[]) {
    Osmium::init(true);

    time_t t0 = time(NULL);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    MyTimerHandler handler;
    infile.read(handler);

    struct tms tms;
    times(&tms);
    std::cout << "user time: " << ((double)tms.tms_utime) / sysconf(_SC_CLK_TCK) << "s   system time: " << ((double)tms.tms_stime) / sysconf(_SC_CLK_TCK) << "s  wallclock time: " << time(NULL) - t0 << "s" << std::endl;
}

