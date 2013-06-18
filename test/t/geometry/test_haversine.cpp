#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <osmium/osm.hpp>
#include <osmium/geometry/haversine.hpp>

#include "test_utils.hpp"

BOOST_AUTO_TEST_SUITE(Haversine)

BOOST_AUTO_TEST_CASE(Haversine) {
    double d = Osmium::Geometry::Haversine::distance(-86.67, 36.12, -118.4, 33.94);
    BOOST_CHECK(d - 2887259.95060711 < 0.001);
}

BOOST_AUTO_TEST_SUITE_END()

