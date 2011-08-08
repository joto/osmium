#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <osmium/osm/position.hpp>

BOOST_AUTO_TEST_SUITE(Position)

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    Osmium::OSM::Position p;
    BOOST_CHECK(!p.defined());
}

BOOST_AUTO_TEST_CASE(instantiation_with_double_parameters) {
    Osmium::OSM::Position p1(1.2, 4.5);
    BOOST_CHECK(p1.defined());
    BOOST_CHECK_EQUAL(p1.x(), 12000000);
    BOOST_CHECK_EQUAL(p1.y(), 45000000);
    BOOST_CHECK_EQUAL(p1.lon(), 1.2);
    BOOST_CHECK_EQUAL(p1.lat(), 4.5);
    Osmium::OSM::Position p2(p1);
    BOOST_CHECK_EQUAL(p2.lat(), 4.5);
    Osmium::OSM::Position p3 = p1;
    BOOST_CHECK_EQUAL(p3.lat(), 4.5);
}

BOOST_AUTO_TEST_CASE(equality) {
    Osmium::OSM::Position p1(1.2, 4.5);
    Osmium::OSM::Position p2(1.2, 4.5);
    Osmium::OSM::Position p3(1.5, 1.5);
    BOOST_CHECK_EQUAL(p1, p2);
    BOOST_CHECK(p1 != p3);
}

BOOST_AUTO_TEST_CASE(output) {
    Osmium::OSM::Position p(-3.2, 47.3);
    output_test_stream out;
    out << p;
    BOOST_CHECK(out.is_equal("(-3.2,47.3)"));
}

BOOST_AUTO_TEST_CASE(conversion_to_uint32_t) {
    Osmium::OSM::Position p1(-180.0, -90.0);
    Osmium::OSM::Position p2(-180.0,  90.0);
    Osmium::OSM::Position p3( 180.0,  90.0);
    Osmium::OSM::Position p4( 180.0, -90.0);
    BOOST_CHECK_EQUAL(64440, static_cast<uint32_t>(p1));
    BOOST_CHECK_EQUAL(    0, static_cast<uint32_t>(p2));
    BOOST_CHECK_EQUAL(  359, static_cast<uint32_t>(p3));
    BOOST_CHECK_EQUAL(64799, static_cast<uint32_t>(p4));
}

BOOST_AUTO_TEST_SUITE_END()

