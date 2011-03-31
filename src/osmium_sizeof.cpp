/*

  This is a small tool to find out the sizes of some basic classes.
  It is only used for Osmium development.

*/

#include <cstdlib>

#include <osmium.hpp>

bool debug;

int main() {
    std::cout << "sizeof(Osmium::OSM::Object)="                   << sizeof(Osmium::OSM::Object) << std::endl;
    std::cout << "sizeof(Osmium::OSM::Node)="                     << sizeof(Osmium::OSM::Node)                     << "  (Object+" << sizeof(Osmium::OSM::Node)                     - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::Way)="                      << sizeof(Osmium::OSM::Way)                      << "  (Object+" << sizeof(Osmium::OSM::Way)                      - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::Relation)="                 << sizeof(Osmium::OSM::Relation)                 << "  (Object+" << sizeof(Osmium::OSM::Relation)                 - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::Multipolygon)="             << sizeof(Osmium::OSM::Multipolygon)             << "  (Object+" << sizeof(Osmium::OSM::Multipolygon)             - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::MultipolygonFromWay)="      << sizeof(Osmium::OSM::MultipolygonFromWay)      << "  (Object+" << sizeof(Osmium::OSM::MultipolygonFromWay)      - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::MultipolygonFromRelation)=" << sizeof(Osmium::OSM::MultipolygonFromRelation) << "  (Object+" << sizeof(Osmium::OSM::MultipolygonFromRelation) - sizeof(Osmium::OSM::Object) << ")" << std::endl;
//    std::cout << "sizeof(Osmium::OSM::WayInfo)="                  << sizeof(Osmium::OSM::WayInfo) << std::endl;
//    std::cout << "sizeof(Osmium::OSM::RingInfo)="                 << sizeof(Osmium::OSM::RingInfo) << std::endl;
}

