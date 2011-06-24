#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <osmium/osm.hpp>
#include <osmium/geometry/point.hpp>

BOOST_AUTO_TEST_SUITE(PointGeometry)

BOOST_AUTO_TEST_CASE(instantiation) {
    Osmium::OSM::Position pos1(1.2, 3.4);
    Osmium::Geometry::Point point1(pos1);
    BOOST_CHECK_EQUAL(point1.lon(), 1.2);
    BOOST_CHECK_EQUAL(point1.lat(), 3.4);
}

BOOST_AUTO_TEST_CASE(output) {
    Osmium::OSM::Position pos1(1.2, 3.4);
    Osmium::Geometry::Point point1(pos1);

    std::ostringstream out1;
    out1 << point1.as_WKT();
    BOOST_CHECK_EQUAL(out1.str(), "POINT(1.2 3.4)");

    std::ostringstream out2;
    out2 << point1.as_EWKT();
    BOOST_CHECK_EQUAL(out2.str(), "SRID=4326;POINT(1.2 3.4)");

    std::ostringstream out3;
    out3 << point1.as_HexWKB();
    BOOST_CHECK_EQUAL(out3.str(), "0101000000333333333333F33F3333333333330B40");
}

BOOST_AUTO_TEST_SUITE_END()

