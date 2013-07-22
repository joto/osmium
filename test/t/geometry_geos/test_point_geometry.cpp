#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/position.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/geometry/geos.hpp>

BOOST_AUTO_TEST_SUITE(GEOS)

BOOST_AUTO_TEST_SUITE(PointGeometry)

BOOST_AUTO_TEST_CASE(instantiation) {
    Osmium::OSM::Position pos1(1.2, 3.4);
    Osmium::Geometry::Point point1(pos1);
    BOOST_CHECK_EQUAL(point1.lon(), 1.2);
    BOOST_CHECK_EQUAL(point1.lat(), 3.4);
}

BOOST_AUTO_TEST_CASE(geos_geometry) {
    Osmium::OSM::Position pos1(1.2, 3.4);
    Osmium::Geometry::Point point1(pos1);

    geos::geom::Point* gp = Osmium::Geometry::create_geos_geometry(point1);
    BOOST_CHECK(gp);
    BOOST_CHECK_EQUAL(gp->getX(), 1.2);
    BOOST_CHECK_EQUAL(gp->getY(), 3.4);
    delete gp;
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
