/*

  This is a small tool to find out the sizes of some basic classes.
  It is only used for Osmium development.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>
#include <osmpbf/osmpbf.h>

#include <osmium/osm/object.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/relations/relation_info.hpp>
#include <osmium/relations/assembler.hpp>

int main() {
    std::cout << "sizeof(Osmium::OSM::Object)="           << sizeof(Osmium::OSM::Object) << "\n";
    std::cout << "sizeof(Osmium::OSM::Node)="             << sizeof(Osmium::OSM::Node)             << "  (Object+" << sizeof(Osmium::OSM::Node)             - sizeof(Osmium::OSM::Object) << ")\n";
    std::cout << "sizeof(Osmium::OSM::Way)="              << sizeof(Osmium::OSM::Way)              << "  (Object+" << sizeof(Osmium::OSM::Way)              - sizeof(Osmium::OSM::Object) << ")\n";
    std::cout << "sizeof(Osmium::OSM::Relation)="         << sizeof(Osmium::OSM::Relation)         << "  (Object+" << sizeof(Osmium::OSM::Relation)         - sizeof(Osmium::OSM::Object) << ")\n";
    std::cout << "sizeof(Osmium::OSM::Area)="             << sizeof(Osmium::OSM::Area)             << "  (Object+" << sizeof(Osmium::OSM::Area)             - sizeof(Osmium::OSM::Object) << ")\n";

    std::cout << "sizeof(Osmium::Relations::RelationInfo)=" << sizeof(Osmium::Relations::RelationInfo) << "\n";
    std::cout << "sizeof(Osmium::Relations::MemberInfo)="   << sizeof(Osmium::Relations::MemberInfo) << "\n";

    std::cout << "sizeof(OSMPBF::BlobHeader)="     << sizeof(OSMPBF::BlobHeader)     << "\n";
    std::cout << "sizeof(OSMPBF::Blob)="           << sizeof(OSMPBF::Blob)           << "\n";
    std::cout << "sizeof(OSMPBF::HeaderBlock)="    << sizeof(OSMPBF::HeaderBlock)    << "\n";
    std::cout << "sizeof(OSMPBF::PrimitiveBlock)=" << sizeof(OSMPBF::PrimitiveBlock) << "\n";
    std::cout << "sizeof(OSMPBF::PrimitiveGroup)=" << sizeof(OSMPBF::PrimitiveGroup) << "\n";
    std::cout << "sizeof(OSMPBF::Node)="           << sizeof(OSMPBF::Node)           << "\n";
    std::cout << "sizeof(OSMPBF::Way)="            << sizeof(OSMPBF::Way)            << "\n";
    std::cout << "sizeof(OSMPBF::Relation)="       << sizeof(OSMPBF::Relation)       << "\n";
    std::cout << "sizeof(OSMPBF::DenseNodes)="     << sizeof(OSMPBF::DenseNodes)     << "\n";
    std::cout << "sizeof(OSMPBF::StringTable)="    << sizeof(OSMPBF::StringTable)    << "\n";
}

