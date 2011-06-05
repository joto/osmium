/*

  This is a small tool to find out the sizes of some basic classes.
  It is only used for Osmium development.

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

#define OSMIUM_MAIN
#include <osmium.hpp>

int main() {
    Osmium::Framework osmium;
    std::cout << "sizeof(Osmium::OSM::Object)="                   << sizeof(Osmium::OSM::Object) << std::endl;
    std::cout << "sizeof(Osmium::OSM::Node)="                     << sizeof(Osmium::OSM::Node)                     << "  (Object+" << sizeof(Osmium::OSM::Node)                     - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::Way)="                      << sizeof(Osmium::OSM::Way)                      << "  (Object+" << sizeof(Osmium::OSM::Way)                      - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::Relation)="                 << sizeof(Osmium::OSM::Relation)                 << "  (Object+" << sizeof(Osmium::OSM::Relation)                 - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::Multipolygon)="             << sizeof(Osmium::OSM::Multipolygon)             << "  (Object+" << sizeof(Osmium::OSM::Multipolygon)             - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::MultipolygonFromWay)="      << sizeof(Osmium::OSM::MultipolygonFromWay)      << "  (Object+" << sizeof(Osmium::OSM::MultipolygonFromWay)      - sizeof(Osmium::OSM::Object) << ")" << std::endl;
    std::cout << "sizeof(Osmium::OSM::MultipolygonFromRelation)=" << sizeof(Osmium::OSM::MultipolygonFromRelation) << "  (Object+" << sizeof(Osmium::OSM::MultipolygonFromRelation) - sizeof(Osmium::OSM::Object) << ")" << std::endl;
//    std::cout << "sizeof(Osmium::OSM::WayInfo)="                  << sizeof(Osmium::OSM::WayInfo) << std::endl;
//    std::cout << "sizeof(Osmium::OSM::RingInfo)="                 << sizeof(Osmium::OSM::RingInfo) << std::endl;

    std::cout << "sizeof(OSMPBF::BlobHeader)="     << sizeof(OSMPBF::BlobHeader)     << std::endl;
    std::cout << "sizeof(OSMPBF::Blob)="           << sizeof(OSMPBF::Blob)           << std::endl;
    std::cout << "sizeof(OSMPBF::HeaderBlock)="    << sizeof(OSMPBF::HeaderBlock)    << std::endl;
    std::cout << "sizeof(OSMPBF::PrimitiveBlock)=" << sizeof(OSMPBF::PrimitiveBlock) << std::endl;
    std::cout << "sizeof(OSMPBF::PrimitiveGroup)=" << sizeof(OSMPBF::PrimitiveGroup) << std::endl;
    std::cout << "sizeof(OSMPBF::Node)="           << sizeof(OSMPBF::Node)           << std::endl;
    std::cout << "sizeof(OSMPBF::Way)="            << sizeof(OSMPBF::Way)            << std::endl;
    std::cout << "sizeof(OSMPBF::Relation)="       << sizeof(OSMPBF::Relation)       << std::endl;
    std::cout << "sizeof(OSMPBF::DenseNodes)="     << sizeof(OSMPBF::DenseNodes)     << std::endl;
    std::cout << "sizeof(OSMPBF::StringTable)="    << sizeof(OSMPBF::StringTable)    << std::endl;
}

