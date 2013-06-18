/*

  This is a small tool to time osmium.

  The code in this example file is released into the Public Domain.

*/

// Set this if you want to report user and system time, too.
// This will not work on Windows systems.
//#define REPORT_USER_AND_SYSTEM_TIME

#include <cstdlib>
#include <time.h>

#ifdef REPORT_USER_AND_SYSTEM_TIME
# include <sys/times.h>
#endif

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>

class MyTimerHandler : public Osmium::Handler::Base {

public:

    MyTimerHandler() : m_nodes(0), m_ways(0), m_relations(0) {
    }

    void node(const shared_ptr<Osmium::OSM::Node const>&) {
        m_nodes++;
    }

    void way(const shared_ptr<Osmium::OSM::Way const>&) {
        m_ways++;
    }

    void relation(const shared_ptr<Osmium::OSM::Relation const>&) {
        m_relations++;
    }

    void final() {
        std::cout << "nodes: " << m_nodes << "  ways: " << m_ways << "  relations: " << m_relations << std::endl;
    }

private:

    uint64_t m_nodes;
    uint64_t m_ways;
    uint64_t m_relations;

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

    std::cout << "wallclock time: " << time(NULL) - t0 << "s" << std::endl;

#ifdef REPORT_USER_AND_SYSTEM_TIME
    struct tms tms;
    times(&tms);
    std::cout << "user time:      " << (static_cast<double>(tms.tms_utime) / sysconf(_SC_CLK_TCK)) << "s" << std::endl;
    std::cout << "system time:    " << (static_cast<double>(tms.tms_stime) / sysconf(_SC_CLK_TCK)) << "s" << std::endl;
#endif

    google::protobuf::ShutdownProtobufLibrary();
}

