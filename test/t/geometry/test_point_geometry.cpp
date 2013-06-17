#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <osmium/osm.hpp>
#include <osmium/geometry/point.hpp>

#include "test_utils.hpp"

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

    std::ostringstream out_wkt;
    out_wkt << point1.as_WKT();
    BOOST_CHECK_EQUAL(out_wkt.str(), "POINT(1.2 3.4)");

    std::ostringstream out_ewkt;
    out_ewkt << point1.as_WKT(true);
    BOOST_CHECK_EQUAL(out_ewkt.str(), "SRID=4326;POINT(1.2 3.4)");

    std::ostringstream out_wkb;
    out_wkb << point1.as_WKB();
    BOOST_CHECK_EQUAL(Osmium::Test::to_hex(out_wkb.str()), "0101000000333333333333F33F3333333333330B40");

    std::ostringstream out_ewkb;
    out_ewkb << point1.as_WKB(true);
    BOOST_CHECK_EQUAL(Osmium::Test::to_hex(out_ewkb.str()), "0101000020E6100000333333333333F33F3333333333330B40");

    std::ostringstream out_hexwkb;
    out_hexwkb << point1.as_HexWKB();
    BOOST_CHECK_EQUAL(out_hexwkb.str(), "0101000000333333333333F33F3333333333330B40");

    std::ostringstream out_hexewkb;
    out_hexewkb << point1.as_HexWKB(true);
    BOOST_CHECK_EQUAL(out_hexewkb.str(), "0101000020E6100000333333333333F33F3333333333330B40");
}

BOOST_AUTO_TEST_SUITE_END()

