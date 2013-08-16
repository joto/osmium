#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/relation_member.hpp>

BOOST_AUTO_TEST_SUITE(RelationMember)

BOOST_AUTO_TEST_CASE(RelationMember_constructor_createsEmptyRelationMember) {
    Osmium::OSM::RelationMember relationmember;

    BOOST_CHECK_EQUAL(relationmember.ref(), 0);
    BOOST_CHECK_EQUAL(relationmember.type(), 'x');
    BOOST_CHECK_EQUAL(relationmember.role(), "");
}

BOOST_AUTO_TEST_CASE(RelationMember_ref_changesRef) {
    Osmium::OSM::RelationMember relationmember;

    relationmember.ref(12);
    BOOST_CHECK_EQUAL(relationmember.ref(), 12);
}

BOOST_AUTO_TEST_CASE(RelationMember_type_changesType) {
    Osmium::OSM::RelationMember relationmember;

    relationmember.type('w');
    BOOST_CHECK_EQUAL(relationmember.type(), 'w');
}

BOOST_AUTO_TEST_CASE(RelationMember_typeName_givesTextualDescriptionOfType) {
    Osmium::OSM::RelationMember relationmember;

    BOOST_CHECK_EQUAL(relationmember.type_name(), "unknown");

    relationmember.type('n');
    BOOST_CHECK_EQUAL(relationmember.type_name(), "node");

    relationmember.type('w');
    BOOST_CHECK_EQUAL(relationmember.type_name(), "way");

    relationmember.type('r');
    BOOST_CHECK_EQUAL(relationmember.type_name(), "relation");

    relationmember.type('a');
    BOOST_CHECK_EQUAL(relationmember.type_name(), "unknown");
}

BOOST_AUTO_TEST_CASE(RelationMember_role_setsRole) {
    Osmium::OSM::RelationMember relationmember;

    relationmember.role("role1");
    BOOST_CHECK_EQUAL(relationmember.role(), "role1");
}

BOOST_AUTO_TEST_CASE(RelationMember_role_throwsIfNameTooLong) {
    // A role, which is too long (more than 256 4-byte characters = 1024 bytes)
    std::string role_too_long("\
00 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
01 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
02 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
03 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
04 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
05 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
06 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
07 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
08 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
09 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
10 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
11 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
12 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
13 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
14 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
15 456789ABCDEF 123456789ABCDEF 123456789ABCDEF 123456789ABCDEF \
xyz");
    Osmium::OSM::RelationMember relationmember;

    BOOST_CHECK_THROW(relationmember.role(role_too_long.c_str());, std::length_error);
}

BOOST_AUTO_TEST_SUITE_END()
