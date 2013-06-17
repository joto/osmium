#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <osmium/osm.hpp>
#include <osmium/geometry/linestring.hpp>

#include "test_utils.hpp"

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

    std::ostringstream out_wkt;
    out_wkt << line1.as_WKT();
    BOOST_CHECK_EQUAL(out_wkt.str(), "LINESTRING(1.9 1.9,1.9 2.9,2.9 2.9)");

    std::ostringstream out_ewkt;
    out_ewkt << line1.as_WKT(true);
    BOOST_CHECK_EQUAL(out_ewkt.str(), "SRID=4326;LINESTRING(1.9 1.9,1.9 2.9,2.9 2.9)");

    std::ostringstream out_wkb;
    out_wkb << line2.as_WKB();
    BOOST_CHECK_EQUAL(Osmium::Test::to_hex(out_wkb.str()), "01020000000300000033333333333307403333333333330740666666666666FE3F3333333333330740666666666666FE3F666666666666FE3F");

    std::ostringstream out_ewkb;
    out_ewkb << line2.as_WKB(true);
    BOOST_CHECK_EQUAL(Osmium::Test::to_hex(out_ewkb.str()), "0102000020E61000000300000033333333333307403333333333330740666666666666FE3F3333333333330740666666666666FE3F666666666666FE3F");

    std::ostringstream out_hexwkb;
    out_hexwkb << line1.as_HexWKB();
    BOOST_CHECK_EQUAL(out_hexwkb.str(), "010200000003000000666666666666FE3F666666666666FE3F666666666666FE3F333333333333074033333333333307403333333333330740");

    std::ostringstream out_hexewkb;
    out_hexewkb << line1.as_HexWKB(true);
    BOOST_CHECK_EQUAL(out_hexewkb.str(), "0102000020E610000003000000666666666666FE3F666666666666FE3F666666666666FE3F333333333333074033333333333307403333333333330740");
}

BOOST_AUTO_TEST_SUITE_END()

