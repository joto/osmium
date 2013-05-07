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

BOOST_AUTO_TEST_CASE(order) {
    Osmium::OSM::Node n1;
    Osmium::OSM::Node n2;
    n1.id(10);
    n1.version(1);
    n2.id(15);
    n2.version(2);
    BOOST_CHECK_EQUAL(true, n1 < n2);
    BOOST_CHECK_EQUAL(false, n1 > n2);
    n1.id(20);
    n1.version(1);
    n2.id(20);
    n2.version(2);
    BOOST_CHECK_EQUAL(true, n1 < n2);
    BOOST_CHECK_EQUAL(false, n1 > n2);
    n1.id(-10);
    n1.version(2);
    n2.id(-15);
    n2.version(1);
    BOOST_CHECK_EQUAL(true, n1 < n2);
    BOOST_CHECK_EQUAL(false, n1 > n2);
}

BOOST_AUTO_TEST_SUITE_END()

