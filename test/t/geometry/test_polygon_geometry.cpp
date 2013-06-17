#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <osmium/osm.hpp>
#include <osmium/geometry/polygon.hpp>

#include "test_utils.hpp"

BOOST_AUTO_TEST_SUITE(PolygonGeometry)

BOOST_AUTO_TEST_CASE(instantiation) {
    Osmium::OSM::WayNodeList wnl;
    Osmium::OSM::WayNode n1(1, Osmium::OSM::Position(1.0, 1.0));
    Osmium::OSM::WayNode n2(2, Osmium::OSM::Position(1.0, 2.0));
    Osmium::OSM::WayNode n3(3, Osmium::OSM::Position(2.0, 2.0));
    Osmium::OSM::WayNode n4(1, Osmium::OSM::Position(1.0, 1.0));
    wnl.add(n1);
    wnl.add(n2);
    wnl.add(n3);
    BOOST_CHECK_THROW(Osmium::Geometry::Polygon polygon(wnl), Osmium::Geometry::RingNotClosed);
    wnl.add(n4); // now its closed
    Osmium::Geometry::Polygon polygon(wnl);
}

BOOST_AUTO_TEST_CASE(output) {
    Osmium::OSM::WayNodeList wnl;
    Osmium::OSM::WayNode n1(1, Osmium::OSM::Position(1.9, 1.9));
    Osmium::OSM::WayNode n2(2, Osmium::OSM::Position(1.9, 2.9));
    Osmium::OSM::WayNode n3(3, Osmium::OSM::Position(2.9, 2.9));
    Osmium::OSM::WayNode n4(1, Osmium::OSM::Position(1.9, 1.9));
    wnl.add(n1);
    wnl.add(n2);
    wnl.add(n3);
    wnl.add(n4);
    Osmium::Geometry::Polygon polygon(wnl);

    std::ostringstream out_wkt;
    out_wkt << polygon.as_WKT();
    BOOST_CHECK_EQUAL(out_wkt.str(), "POLYGON((1.9 1.9,1.9 2.9,2.9 2.9,1.9 1.9))");

    std::ostringstream out_ewkt;
    out_ewkt << polygon.as_WKT(true);
    BOOST_CHECK_EQUAL(out_ewkt.str(), "SRID=4326;POLYGON((1.9 1.9,1.9 2.9,2.9 2.9,1.9 1.9))");

    std::ostringstream out_wkb;
    out_wkb << polygon.as_WKB();
    BOOST_CHECK_EQUAL(Osmium::Test::to_hex(out_wkb.str()), "01030000000100000004000000666666666666FE3F666666666666FE3F666666666666FE3F333333333333074033333333333307403333333333330740666666666666FE3F666666666666FE3F");

    std::ostringstream out_ewkb;
    out_ewkb << polygon.as_WKB(true);
    BOOST_CHECK_EQUAL(Osmium::Test::to_hex(out_ewkb.str()), "0103000020E61000000100000004000000666666666666FE3F666666666666FE3F666666666666FE3F333333333333074033333333333307403333333333330740666666666666FE3F666666666666FE3F");

    std::ostringstream out_hexwkb;
    out_hexwkb << polygon.as_HexWKB();
    BOOST_CHECK_EQUAL(out_hexwkb.str(), "01030000000100000004000000666666666666FE3F666666666666FE3F666666666666FE3F333333333333074033333333333307403333333333330740666666666666FE3F666666666666FE3F");

    std::ostringstream out_hexewkb;
    out_hexewkb << polygon.as_HexWKB(true);
    BOOST_CHECK_EQUAL(out_hexewkb.str(), "0103000020E61000000100000004000000666666666666FE3F666666666666FE3F666666666666FE3F333333333333074033333333333307403333333333330740666666666666FE3F666666666666FE3F");
}

BOOST_AUTO_TEST_SUITE_END()

