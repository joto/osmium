/*

  This is a small tool to find out the sizes of some basic classes.
  It is only used for Osmium development.

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

#include <cstdlib>

#include <osmium.hpp>
#include <osmium/output/xml.hpp>

int main() {
    Osmium::init();

    std::cout << "sizeof(Osmium::OSM::Object)="           << sizeof(Osmium::OSM::Object) << "\n";
    std::cout << "sizeof(Osmium::OSM::Node)="             << sizeof(Osmium::OSM::Node)             << "  (Object+" << sizeof(Osmium::OSM::Node)             - sizeof(Osmium::OSM::Object) << ")\n";
    std::cout << "sizeof(Osmium::OSM::Way)="              << sizeof(Osmium::OSM::Way)              << "  (Object+" << sizeof(Osmium::OSM::Way)              - sizeof(Osmium::OSM::Object) << ")\n";
    std::cout << "sizeof(Osmium::OSM::Relation)="         << sizeof(Osmium::OSM::Relation)         << "  (Object+" << sizeof(Osmium::OSM::Relation)         - sizeof(Osmium::OSM::Object) << ")\n";
    std::cout << "sizeof(Osmium::OSM::Area)="             << sizeof(Osmium::OSM::Area)             << "  (Object+" << sizeof(Osmium::OSM::Area)             - sizeof(Osmium::OSM::Object) << ")\n";
    std::cout << "sizeof(Osmium::OSM::AreaFromWay)="      << sizeof(Osmium::OSM::AreaFromWay)      << "  (Object+" << sizeof(Osmium::OSM::AreaFromWay)      - sizeof(Osmium::OSM::Object) << ")\n";
    std::cout << "sizeof(Osmium::OSM::AreaFromRelation)=" << sizeof(Osmium::OSM::AreaFromRelation) << "  (Object+" << sizeof(Osmium::OSM::AreaFromRelation) - sizeof(Osmium::OSM::Object) << ")\n";
//    std::cout << "sizeof(Osmium::OSM::WayInfo)="                  << sizeof(Osmium::OSM::WayInfo) << "\n";
//    std::cout << "sizeof(Osmium::OSM::RingInfo)="                 << sizeof(Osmium::OSM::RingInfo) << "\n";

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

    std::cout << "sizeof(Osmium::Output::PBF)="    << sizeof(Osmium::Output::PBF)    << "\n";
    std::cout << "sizeof(Osmium::Output::XML)="    << sizeof(Osmium::Output::XML)    << "\n";
}

