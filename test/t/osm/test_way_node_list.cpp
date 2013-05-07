#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/way_node_list.hpp>

BOOST_AUTO_TEST_SUITE(WayNodeList)

BOOST_AUTO_TEST_CASE(set_position) {
    Osmium::OSM::WayNodeList wnl;
    BOOST_CHECK_EQUAL(wnl.size(), 0);
    BOOST_CHECK(!wnl.has_position());
    Osmium::OSM::WayNode wn(5);
    wnl.add(wn);
    BOOST_CHECK_EQUAL(wnl.size(), 1);
    BOOST_CHECK(!wnl.has_position());
    BOOST_CHECK_EQUAL(wnl[0].ref(), 5);
    wnl.add(17);
    BOOST_CHECK_EQUAL(wnl.size(), 2);
    BOOST_CHECK_EQUAL(wnl[1].ref(), 17);
    wnl.clear();
    BOOST_CHECK_EQUAL(wnl.size(), 0);
}

BOOST_AUTO_TEST_CASE(closed_or_not) {
    Osmium::OSM::WayNodeList wnl;
    wnl.add(5);
    wnl.add(7);
    wnl.add(8);
    BOOST_CHECK(!wnl.is_closed());
    wnl.add(5);
    BOOST_CHECK(wnl.is_closed());
}

BOOST_AUTO_TEST_SUITE_END()

