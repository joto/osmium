#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <osmium/osm/position.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/geometry/linestring.hpp>
#include <osmium/geometry/polygon.hpp>
#include <osmium/geometry/ogr.hpp>

BOOST_AUTO_TEST_SUITE(Geometry)

BOOST_AUTO_TEST_CASE(point_from_position) {
    Osmium::OSM::Position pos(1.2, 3.4);
    Osmium::Geometry::Point point(pos);
    BOOST_CHECK_EQUAL(point.lon(), 1.2);
    BOOST_CHECK_EQUAL(point.lat(), 3.4);
    OGRPoint* ogrpoint = Osmium::Geometry::create_ogr_geometry(point);
    BOOST_CHECK_EQUAL(ogrpoint->getX(), 1.2);
    BOOST_CHECK_EQUAL(ogrpoint->getY(), 3.4);
    delete ogrpoint;
}

BOOST_AUTO_TEST_CASE(linestring_from_way) {
    Osmium::OSM::Position pos1(1.2, 3.4);
    Osmium::OSM::Position pos2(2.3, 4.5);
    Osmium::OSM::WayNodeList wnl;
    wnl.add(Osmium::OSM::WayNode(1, pos1));
    wnl.add(Osmium::OSM::WayNode(2, pos2));
    Osmium::Geometry::LineString linestring(wnl);
    OGRLineString* ogrlinestring = Osmium::Geometry::create_ogr_geometry(linestring);
    OGRPoint ogrpoint;
    ogrlinestring->StartPoint(&ogrpoint);
    BOOST_CHECK_EQUAL(ogrpoint.getX(), 1.2);
    BOOST_CHECK_EQUAL(ogrpoint.getY(), 3.4);
    ogrlinestring->getPoint(1, &ogrpoint);
    BOOST_CHECK_EQUAL(ogrpoint.getX(), 2.3);
    BOOST_CHECK_EQUAL(ogrpoint.getY(), 4.5);
    delete ogrlinestring;
}

BOOST_AUTO_TEST_CASE(polygon_from_way) {
    Osmium::OSM::Position pos1(1.2, 3.4);
    Osmium::OSM::Position pos2(2.3, 4.5);
    Osmium::OSM::WayNodeList wnl;
    wnl.add(Osmium::OSM::WayNode(1, pos1));
    wnl.add(Osmium::OSM::WayNode(2, pos2));
    wnl.add(Osmium::OSM::WayNode(1, pos1));
    Osmium::Geometry::Polygon polygon(wnl);
    OGRPolygon* ogrpolygon = Osmium::Geometry::create_ogr_geometry(polygon);
    std::string ogrwkb;
    ogrwkb.resize(ogrpolygon->WkbSize());
    ogrpolygon->exportToWkb(wkbNDR, (unsigned char*)ogrwkb.c_str());
    output_test_stream osmiumwkb;
    osmiumwkb << polygon.as_WKB();
    BOOST_CHECK_EQUAL(osmiumwkb.str().size(), ogrpolygon->WkbSize());
    BOOST_CHECK(osmiumwkb.is_equal(ogrwkb));
    delete ogrpolygon;
}

BOOST_AUTO_TEST_SUITE_END()

