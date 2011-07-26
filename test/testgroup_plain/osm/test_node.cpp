#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/node.hpp>

BOOST_AUTO_TEST_SUITE(Node)

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    Osmium::OSM::Node n;
    BOOST_CHECK_EQUAL(0, n.id());
    BOOST_CHECK_EQUAL(-1, n.uid());
}

BOOST_AUTO_TEST_SUITE_END()

