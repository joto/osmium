#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <osmium/osm.hpp>
#include <osmium/geometry/linestring.hpp>

BOOST_AUTO_TEST_SUITE(LineStringGeometry)

BOOST_AUTO_TEST_CASE(instantiation) {
    Osmium::OSM::WayNodeList wnl;
    Osmium::OSM::WayNode n1(1, Osmium::OSM::Position(1.0, 1.0));
    Osmium::OSM::WayNode n2(2, Osmium::OSM::Position(1.0, 2.0));
    Osmium::OSM::WayNode n3(3, Osmium::OSM::Position(2.0, 2.0));
    wnl.add(n1);
    wnl.add(n2);
    wnl.add(n3);
    Osmium::Geometry::LineString line1(wnl);
}

BOOST_AUTO_TEST_CASE(output) {
    Osmium::OSM::WayNodeList wnl;
    Osmium::OSM::WayNode n1(1, Osmium::OSM::Position(1.9, 1.9));
    Osmium::OSM::WayNode n2(2, Osmium::OSM::Position(1.9, 2.9));
    Osmium::OSM::WayNode n3(3, Osmium::OSM::Position(2.9, 2.9));
    wnl.add(n1);
    wnl.add(n2);
    wnl.add(n3);
    Osmium::Geometry::LineString line1(wnl);
    Osmium::Geometry::LineString line2(wnl, true);

    std::ostringstream out1;
    out1 << line1.as_WKT();
    BOOST_CHECK_EQUAL(out1.str(), "LINESTRING(1.9 1.9,1.9 2.9,2.9 2.9)");

    std::ostringstream out2;
    out2 << line1.as_EWKT();
    BOOST_CHECK_EQUAL(out2.str(), "SRID=4326;LINESTRING(1.9 1.9,1.9 2.9,2.9 2.9)");

    std::ostringstream out3;
    out3 << line2.as_WKT();
    BOOST_CHECK_EQUAL(out3.str(), "LINESTRING(2.9 2.9,1.9 2.9,1.9 1.9)");

}

BOOST_AUTO_TEST_SUITE_END()

