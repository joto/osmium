#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/relation_member_list.hpp>

BOOST_AUTO_TEST_SUITE(RelationMemberList)

struct FilledRelationMemberListFixture {
    FilledRelationMemberListFixture() {
        relationmemberlist.add_member('n', 1, "role1");
        relationmemberlist.add_member('w', 2, "role2");
        relationmemberlist.add_member('r', 3, "role3");
    }

    Osmium::OSM::RelationMemberList relationmemberlist;
};

BOOST_AUTO_TEST_CASE(RelationMemberList_constructor_createsEmptyList) {
    Osmium::OSM::RelationMemberList relationmemberlist;

    BOOST_CHECK_EQUAL(relationmemberlist.size(), 0);
}

BOOST_AUTO_TEST_CASE(RelationMemberList_size_returnsNumberOfElements) {
    Osmium::OSM::RelationMemberList relationmemberlist;

    BOOST_CHECK_EQUAL(relationmemberlist.size(), 0);

    relationmemberlist.add_member('n', 1, "role1");
    BOOST_CHECK_EQUAL(relationmemberlist.size(), 1);

    relationmemberlist.add_member('w', 2, "role2");
    BOOST_CHECK_EQUAL(relationmemberlist.size(), 2);
}

BOOST_AUTO_TEST_CASE(RelationMemberList_clear_clearsList) {
    FilledRelationMemberListFixture fix;

    fix.relationmemberlist.clear();
    BOOST_CHECK_EQUAL(fix.relationmemberlist.size(), 0);
}

BOOST_AUTO_TEST_CASE(RelationMemberList_bracketOperator_accessesElements) {
    FilledRelationMemberListFixture fix;

    BOOST_CHECK_EQUAL(fix.relationmemberlist[0].ref(), 1);
    BOOST_CHECK_EQUAL(fix.relationmemberlist[1].ref(), 2);
    BOOST_CHECK_EQUAL(fix.relationmemberlist[2].ref(), 3);
}

BOOST_AUTO_TEST_CASE(RelationMemberList_constBracketOperator_accesesElements) {
    FilledRelationMemberListFixture fix;
    const Osmium::OSM::RelationMemberList constrelationmemberlist = fix.relationmemberlist;

    BOOST_CHECK_EQUAL(constrelationmemberlist[0].ref(), 1);
    BOOST_CHECK_EQUAL(constrelationmemberlist[1].ref(), 2);
    BOOST_CHECK_EQUAL(constrelationmemberlist[2].ref(), 3);
}

BOOST_AUTO_TEST_CASE(RelationMemberList_iterator_iteratesOverList) {
    FilledRelationMemberListFixture fix;
    Osmium::OSM::RelationMemberList::iterator it;

    it = fix.relationmemberlist.begin();
    BOOST_CHECK_EQUAL(it->ref(), 1);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 2);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 3);
    ++it;
    BOOST_CHECK_EQUAL(it == fix.relationmemberlist.end(), true);
}

BOOST_AUTO_TEST_CASE(RelationMemberList_constIterator_iteratesOverList) {
    FilledRelationMemberListFixture fix;
    const Osmium::OSM::RelationMemberList constrelationmemberlist = fix.relationmemberlist;
    Osmium::OSM::RelationMemberList::const_iterator it;

    it = constrelationmemberlist.begin();
    BOOST_CHECK_EQUAL(it->ref(), 1);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 2);
    ++it;
    BOOST_CHECK_EQUAL(it->ref(), 3);
    ++it;
    BOOST_CHECK_EQUAL(it == constrelationmemberlist.end(), true);
}

BOOST_AUTO_TEST_CASE(RelationMemberList_addMember_addsNewMember) {
    Osmium::OSM::RelationMemberList relationmemberlist;

    relationmemberlist.add_member('n', 1, "role1");
    BOOST_CHECK_EQUAL(relationmemberlist[0].type(), 'n');
    BOOST_CHECK_EQUAL(relationmemberlist[0].ref(), 1);
    BOOST_CHECK_EQUAL(relationmemberlist[0].role(), "role1");
}

BOOST_AUTO_TEST_SUITE_END()
