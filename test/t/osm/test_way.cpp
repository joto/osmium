#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/way.hpp>

BOOST_AUTO_TEST_SUITE(Way)

struct FilledWayFixture {
    FilledWayFixture() {
        way.id(123);
        way.add_node(1);
        way.add_node(2);
        way.add_node(3);
    }

    Osmium::OSM::Way way;
};

void check_wayNodeLists_for_equality(const Osmium::OSM::WayNodeList& wnl1, const Osmium::OSM::WayNodeList& wnl2) {
    Osmium::OSM::WayNodeList::const_iterator it1 = wnl1.begin();
    Osmium::OSM::WayNodeList::const_iterator it2 = wnl2.begin();

    while ((it1 != wnl1.end()) && (it2 != wnl2.end())) {
        BOOST_CHECK_EQUAL(it1->ref(), it2->ref());
        ++it1;
        ++it2;
    }
    BOOST_CHECK((it1 == wnl1.end()) && (it2 == wnl2.end()));
}

BOOST_AUTO_TEST_CASE(Way_constructor_createsEmptyWay) {
    Osmium::OSM::Way way;

    BOOST_CHECK_EQUAL(way.nodes().empty(), true);
}

BOOST_AUTO_TEST_CASE(Way_copyConstructor_copiesWayNodes) {
    FilledWayFixture fix;
    Osmium::OSM::Way copiedWay(fix.way);

    check_wayNodeLists_for_equality(fix.way.nodes(), copiedWay.nodes());
    BOOST_CHECK_EQUAL(fix.way.id(), copiedWay.id());
}

BOOST_AUTO_TEST_CASE(Way_nodes_returnsNodeList) {
    FilledWayFixture fix;

    BOOST_CHECK_EQUAL(fix.way.nodes()[0].ref(), 1);
}

BOOST_AUTO_TEST_CASE(Way_constNodes_returnsNodeList) {
    FilledWayFixture fix;
    const Osmium::OSM::Way const_way = fix.way;

    BOOST_CHECK_EQUAL(const_way.nodes()[0].ref(), 1);
}

BOOST_AUTO_TEST_CASE(Way_type_returnsWayType) {
    Osmium::OSM::Way way;

    BOOST_CHECK_EQUAL(way.type(), WAY);
}

BOOST_AUTO_TEST_CASE(Way_getNodeId_returnsIdOfNode) {
    FilledWayFixture fix;

    BOOST_CHECK_EQUAL(fix.way.get_node_id(1), 2);
}

BOOST_AUTO_TEST_CASE(Way_addNode_addsNode) {
    Osmium::OSM::Way way;

    way.add_node(12);
    BOOST_CHECK_EQUAL(way.nodes()[0].ref(), 12);
}

BOOST_AUTO_TEST_CASE(Way_getFirstNodeId_returnsIdOfFirstNode) {
    FilledWayFixture fix;

    BOOST_CHECK_EQUAL(fix.way.get_first_node_id(), 1);
}

BOOST_AUTO_TEST_CASE(Way_getLastNodeId_returnsIdOfLastNode) {
    FilledWayFixture fix;

    BOOST_CHECK_EQUAL(fix.way.get_last_node_id(), 3);
}

BOOST_AUTO_TEST_CASE(Way_isClosed_detectsIfWayIsClosed) {
    FilledWayFixture fix;

    BOOST_CHECK_EQUAL(fix.way.is_closed(), false);
    fix.way.add_node(1);
    BOOST_CHECK_EQUAL(fix.way.is_closed(), true);
}

BOOST_AUTO_TEST_CASE(Way_comparisonOperator_comparesByIdThenByVersion) {
    Osmium::OSM::Way way1, way2;

    BOOST_CHECK_EQUAL(way1 < way2, false);
    BOOST_CHECK_EQUAL(way1 > way2, false);

    way1.id(12);
    way2.id(10);
    BOOST_CHECK_EQUAL(way1 < way2, false);
    BOOST_CHECK_EQUAL(way1 > way2, true);

    way1.id(12);
    way2.id(12);
    way1.version(1);
    way2.version(2);
    BOOST_CHECK_EQUAL(way1 < way2, true);
    BOOST_CHECK_EQUAL(way1 > way2, false);
}

BOOST_AUTO_TEST_SUITE_END()
