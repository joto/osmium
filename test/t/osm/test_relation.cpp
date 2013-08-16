#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/relation.hpp>

BOOST_AUTO_TEST_SUITE(Relation)

struct FilledRelationFixture {
    FilledRelationFixture() {
        rel.id(123);
        rel.add_member('n', 1, "role1");
        rel.add_member('w', 2, "role2");
        rel.add_member('r', 3, "role3");
    }

    Osmium::OSM::Relation rel;
};

void check_relationMemberLists_for_equaltiy(const Osmium::OSM::RelationMemberList& rml1, const Osmium::OSM::RelationMemberList& rml2) {
    Osmium::OSM::RelationMemberList::const_iterator it1 = rml1.begin();
    Osmium::OSM::RelationMemberList::const_iterator it2 = rml2.begin();

    while ((it1 != rml1.end()) && (it2 != rml2.end())) {
        BOOST_CHECK_EQUAL(it1->ref(), it2->ref());
        BOOST_CHECK_EQUAL(it1->type(), it2->type());
        BOOST_CHECK_EQUAL(it1->role(), it2->role());
        ++it1;
        ++it2;
    }
    BOOST_CHECK((it1 == rml1.end()) && (it2 == rml2.end()));
}

BOOST_AUTO_TEST_CASE(Relation_constructor_createsEmptyRelation) {
    Osmium::OSM::Relation rel;

    BOOST_CHECK_EQUAL(rel.members().size(), 0u);
}

BOOST_AUTO_TEST_CASE(Relation_copyConstructor_copiesRelationContents) {
    FilledRelationFixture fix;
    Osmium::OSM::Relation copiedRel(fix.rel);

    check_relationMemberLists_for_equaltiy(fix.rel.members(), copiedRel.members());
    BOOST_CHECK_EQUAL(fix.rel.id(), copiedRel.id());
}

BOOST_AUTO_TEST_CASE(Relation_members_accesesMembers) {
    FilledRelationFixture fix;

    BOOST_CHECK_EQUAL(fix.rel.members()[0].ref(), 1);
    BOOST_CHECK_EQUAL(fix.rel.members()[0].role(), "role1");
    BOOST_CHECK_EQUAL(fix.rel.members()[1].ref(), 2);
    BOOST_CHECK_EQUAL(fix.rel.members()[1].role(), "role2");
}

BOOST_AUTO_TEST_CASE(Relation_type_returnsRelationType) {
    Osmium::OSM::Relation rel;

    BOOST_CHECK_EQUAL(rel.type(), RELATION);
}

BOOST_AUTO_TEST_CASE(Relation_addMember_addsMember) {
    Osmium::OSM::Relation rel;

    rel.add_member('n', 1, "role1");
    BOOST_CHECK_EQUAL(rel.members()[0].type(), 'n');
    BOOST_CHECK_EQUAL(rel.members()[0].ref(), 1);
    BOOST_CHECK_EQUAL(rel.members()[0].role(), "role1");
}

BOOST_AUTO_TEST_CASE(Relation_getMember_returnsPointerToMember) {
    FilledRelationFixture fix;

    BOOST_CHECK_EQUAL(fix.rel.get_member(0)->ref(), 1);
    BOOST_CHECK_EQUAL(fix.rel.get_member(1)->ref(), 2);
    BOOST_CHECK_EQUAL(fix.rel.get_member(4) == NULL, true);
}

BOOST_AUTO_TEST_CASE(Relation_comparisonOperator_comparesByIdThenByVersion) {
    Osmium::OSM::Relation rel1, rel2;

    BOOST_CHECK_EQUAL(rel1 < rel2, false);
    BOOST_CHECK_EQUAL(rel1 > rel2, false);

    rel1.id(12);
    rel2.id(10);
    BOOST_CHECK_EQUAL(rel1 < rel2, false);
    BOOST_CHECK_EQUAL(rel1 > rel2, true);

    rel1.id(12);
    rel2.id(12);
    rel1.version(1);
    rel2.version(2);
    BOOST_CHECK_EQUAL(rel1 < rel2, true);
    BOOST_CHECK_EQUAL(rel1 > rel2, false);
}

BOOST_AUTO_TEST_SUITE_END()
