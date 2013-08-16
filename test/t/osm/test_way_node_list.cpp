#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/way_node_list.hpp>

BOOST_AUTO_TEST_SUITE(WayNodeList)

struct FilledWayNodeListFixture {
    FilledWayNodeListFixture() {
        wnl.add(1);
        wnl.add(2);
        wnl.add(3);
    }

    Osmium::OSM::WayNodeList wnl;
};

BOOST_AUTO_TEST_CASE(set_position) {
    Osmium::OSM::WayNodeList wnl;
    BOOST_CHECK_EQUAL(wnl.size(), 0u);
    BOOST_CHECK(!wnl.has_position());
    Osmium::OSM::WayNode wn(5);
    wnl.add(wn);
    BOOST_CHECK_EQUAL(wnl.size(), 1u);
    BOOST_CHECK(!wnl.has_position());
    BOOST_CHECK_EQUAL(wnl[0].ref(), 5);
    wnl.add(17);
    BOOST_CHECK_EQUAL(wnl.size(), 2u);
    BOOST_CHECK_EQUAL(wnl[1].ref(), 17);
    wnl.clear();
    BOOST_CHECK_EQUAL(wnl.size(), 0u);
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

BOOST_AUTO_TEST_CASE(WayNodeList_constructor_createsEmptyList) {
    Osmium::OSM::WayNodeList wnl;

    BOOST_CHECK_EQUAL(wnl.empty(), true);
}

BOOST_AUTO_TEST_CASE(WayNodeList_size_returnsCorrectSize) {
    Osmium::OSM::WayNodeList wnl;

    BOOST_CHECK_EQUAL(wnl.size(), 0u);

    wnl.add(1);
    BOOST_CHECK_EQUAL(wnl.size(), 1u);

    wnl.add(2);
    BOOST_CHECK_EQUAL(wnl.size(), 2u);
}

BOOST_AUTO_TEST_CASE(WayNodeList_empty_returnsIfListIsEmpty) {
    Osmium::OSM::WayNodeList wnl;

    BOOST_CHECK_EQUAL(wnl.empty(), true);

    wnl.add(1);
    BOOST_CHECK_EQUAL(wnl.empty(), false);
}

BOOST_AUTO_TEST_CASE(WayNodeList_clear_clearsList) {
    FilledWayNodeListFixture fix;

    fix.wnl.clear();
    BOOST_CHECK_EQUAL(fix.wnl.empty(), true);
}

BOOST_AUTO_TEST_CASE(WayNodeList_iterator_traversesList) {
    FilledWayNodeListFixture fix;

    Osmium::OSM::WayNodeList::iterator it = fix.wnl.begin();

    BOOST_CHECK_EQUAL(it->ref(), 1);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 2);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 3);
    ++it;
    BOOST_CHECK_EQUAL(it == fix.wnl.end(), true);
}

BOOST_AUTO_TEST_CASE(WayNodeList_constiterator_traversesList) {
    FilledWayNodeListFixture fix;

    const Osmium::OSM::WayNodeList const_wnl = fix.wnl;
    Osmium::OSM::WayNodeList::const_iterator it = const_wnl.begin();

    BOOST_CHECK_EQUAL(it->ref(), 1);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 2);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 3);
    ++it;
    BOOST_CHECK_EQUAL(it == const_wnl.end(), true);
}

BOOST_AUTO_TEST_CASE(WayNodeList_reverseIterator_traversesList) {
    FilledWayNodeListFixture fix;

    Osmium::OSM::WayNodeList::reverse_iterator it = fix.wnl.rbegin();

    BOOST_CHECK_EQUAL(it->ref(), 3);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 2);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 1);
    ++it;
    BOOST_CHECK_EQUAL(it == fix.wnl.rend(), true);
}

BOOST_AUTO_TEST_CASE(WayNodeList_constReverseIterator_traversesList) {
    FilledWayNodeListFixture fix;

    const Osmium::OSM::WayNodeList const_wnl = fix.wnl;
    Osmium::OSM::WayNodeList::const_reverse_iterator it = const_wnl.rbegin();

    BOOST_CHECK_EQUAL(it->ref(), 3);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 2);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 1);
    ++it;
    BOOST_CHECK_EQUAL(it == const_wnl.rend(), true);
}

// XXX: don't know how insert should work

BOOST_AUTO_TEST_CASE(WayNodeList_bracket_accessesItems) {
    FilledWayNodeListFixture fix;

    BOOST_CHECK_EQUAL(fix.wnl[0].ref(), 1);
    BOOST_CHECK_EQUAL(fix.wnl[1].ref(), 2);
}

BOOST_AUTO_TEST_CASE(WayNodeList_constBracket_accessesItems) {
    FilledWayNodeListFixture fix;
    const Osmium::OSM::WayNodeList const_wnl = fix.wnl;

    BOOST_CHECK_EQUAL(const_wnl[0].ref(), 1);
    BOOST_CHECK_EQUAL(const_wnl[1].ref(), 2);
}

BOOST_AUTO_TEST_CASE(WayNodeList_front_returnsFirstElement) {
    FilledWayNodeListFixture fix;

    BOOST_CHECK_EQUAL(fix.wnl.front().ref(), 1);
}

BOOST_AUTO_TEST_CASE(WayNodeList_constFront_returnsFirstElement) {
    FilledWayNodeListFixture fix;
    const Osmium::OSM::WayNodeList const_wnl = fix.wnl;

    BOOST_CHECK_EQUAL(const_wnl.front().ref(), 1);
}

BOOST_AUTO_TEST_CASE(WayNodeList_back_returnsLastElement) {
    FilledWayNodeListFixture fix;

    BOOST_CHECK_EQUAL(fix.wnl.back().ref(), 3);
}

BOOST_AUTO_TEST_CASE(WayNodeList_constBack_returnsLastElement) {
    FilledWayNodeListFixture fix;
    const Osmium::OSM::WayNodeList const_wnl = fix.wnl;


    BOOST_CHECK_EQUAL(const_wnl.back().ref(), 3);
}

BOOST_AUTO_TEST_CASE(WayNodeList_pushBack_insertsAtTheEnd) {
    FilledWayNodeListFixture fix;

    fix.wnl.push_back(Osmium::OSM::WayNode(12));

    BOOST_CHECK_EQUAL(fix.wnl.back().ref(), 12);
}

BOOST_AUTO_TEST_CASE(WayNodeList_pushBackWithRef_insertsAtTheEnd) {
    FilledWayNodeListFixture fix;

    fix.wnl.push_back(12);

    BOOST_CHECK_EQUAL(fix.wnl.back().ref(), 12);
}

BOOST_AUTO_TEST_SUITE_END()

