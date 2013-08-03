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

BOOST_AUTO_TEST_CASE(order_for_pointers) {
    shared_ptr<Osmium::OSM::Node> ptr1 = make_shared<Osmium::OSM::Node>();
    shared_ptr<Osmium::OSM::Node> ptr2 = make_shared<Osmium::OSM::Node>();

    ptr1->id(10);
    ptr1->version(1);
    ptr2->id(15);
    ptr2->version(2);

    BOOST_CHECK_EQUAL(true, ptr1 < ptr2);
    shared_ptr<Osmium::OSM::Node const> ptr1a = ptr1;
    shared_ptr<Osmium::OSM::Node const> ptr2a = ptr2;
    BOOST_CHECK_EQUAL(true, ptr1a < ptr2a);
    //BOOST_CHECK_EQUAL(false, ptr1a > ptr2a);

    ptr2->id(20);
    ptr2->version(1);
    ptr1->id(20);
    ptr1->version(2);
    //BOOST_CHECK_EQUAL(false, ptr1 < ptr2);
    //BOOST_CHECK_EQUAL(false, ptr1 > ptr2);
    ptr1->id(-10);
    ptr1->version(2);
    ptr2->id(-15);
    ptr2->version(1);
    BOOST_CHECK_EQUAL(true, ptr1 < ptr2);
    //BOOST_CHECK_EQUAL(false, ptr1 > ptr2);
}

BOOST_AUTO_TEST_CASE(Node_position_setsPosition) {
    Osmium::OSM::Node node;
    Osmium::OSM::Position example_position(83902210,490096164);

    node.position(example_position);
    BOOST_CHECK_EQUAL(node.position().x(), 83902210);
    BOOST_CHECK_EQUAL(node.position().y(), 490096164);
}

BOOST_AUTO_TEST_CASE(Node_type_returnsNodeType) {
    Osmium::OSM::Node node;

    BOOST_CHECK_EQUAL(node.type(), NODE);
}

BOOST_AUTO_TEST_CASE(Node_lonlatGetter_convertPositionToDouble) {
    Osmium::OSM::Node node;
    Osmium::OSM::Position example_position(83902210,490096164);

    node.position(example_position);
    BOOST_CHECK_CLOSE(node.lon(), 8.390221,   0.000000001);
    BOOST_CHECK_CLOSE(node.lat(), 49.0096164, 0.000000001);
}

BOOST_AUTO_TEST_CASE(Node_lonlatSetter_convertPositionToInt) {
    Osmium::OSM::Node node;
    node.lon(8.390221);
    node.lat(49.0096164);

    BOOST_CHECK_EQUAL(node.position().x(), 83902210);
    BOOST_CHECK_EQUAL(node.position().y(), 490096164);
}

BOOST_AUTO_TEST_SUITE_END()

