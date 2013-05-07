#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/way_node.hpp>

BOOST_AUTO_TEST_SUITE(WayNode)

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    Osmium::OSM::WayNode wn;
    BOOST_CHECK_EQUAL(wn.ref(), 0);
    BOOST_CHECK(!wn.has_position());
}

BOOST_AUTO_TEST_CASE(instantiation_with_id) {
    Osmium::OSM::WayNode wn(7);
    BOOST_CHECK_EQUAL(wn.ref(), 7);
}

BOOST_AUTO_TEST_CASE(equality) {
    Osmium::OSM::WayNode wn1(7);
    Osmium::OSM::WayNode wn2(7);
    Osmium::OSM::WayNode wn3(9);
    BOOST_CHECK(wn1 == wn2);
    BOOST_CHECK(wn1 != wn3);
}

BOOST_AUTO_TEST_CASE(set_position) {
    Osmium::OSM::WayNode wn(7);
    BOOST_CHECK_EQUAL(wn.position(), Osmium::OSM::Position());
    wn.position(Osmium::OSM::Position(13.5, -7.2));
    BOOST_CHECK_EQUAL(wn.position().lon(), 13.5);
    BOOST_CHECK(wn.has_position());
}

BOOST_AUTO_TEST_SUITE_END()

