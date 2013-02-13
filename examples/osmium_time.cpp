/*

  This is a small tool to time osmium.

  The code in this example file is released into the Public Domain.

*/

#include <cstdlib>
#include <time.h>
#include <sys/times.h>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

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

int main(int argc, char* argv[]) {
    time_t t0 = time(NULL);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE" << std::endl;
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    MyTimerHandler handler;
    Osmium::Input::read(infile, handler);

    struct tms tms;
    times(&tms);
    std::cout << "user time: " << ((double)tms.tms_utime) / sysconf(_SC_CLK_TCK) << "s   system time: " << ((double)tms.tms_stime) / sysconf(_SC_CLK_TCK) << "s  wallclock time: " << time(NULL) - t0 << "s" << std::endl;

    google::protobuf::ShutdownProtobufLibrary();
}

