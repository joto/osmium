#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <inttypes.h>

#include <osmium/osm/tag_list.hpp>

BOOST_AUTO_TEST_SUITE(TagList)

BOOST_AUTO_TEST_CASE(TagList_constructor_createsEmptyList) {
    Osmium::OSM::TagList taglist;

    BOOST_CHECK_EQUAL(taglist.empty(), true);
}

BOOST_AUTO_TEST_CASE(TagList_size_returnsNumberOfElements) {
    Osmium::OSM::TagList taglist;

    BOOST_CHECK_EQUAL(taglist.size(), 0);

    taglist.add("entry1", "value1");
    BOOST_CHECK_EQUAL(taglist.size(), 1);

    taglist.add("entry2", "value2");
    BOOST_CHECK_EQUAL(taglist.size(), 2);
}

BOOST_AUTO_TEST_CASE(TagList_empty_returnsTrueOnEmptyList) {
    Osmium::OSM::TagList taglist;

    BOOST_CHECK_EQUAL(taglist.empty(), true);

    taglist.add("entry1", "value1");
    BOOST_CHECK_EQUAL(taglist.empty(), false);
}

BOOST_AUTO_TEST_CASE(TagList_clear_clearsList) {
    Osmium::OSM::TagList taglist;

    taglist.add("entry1", "value1");
    taglist.clear();

    BOOST_CHECK_EQUAL(taglist.empty(), true);
}

BOOST_AUTO_TEST_CASE(TagList_bracketOperator_accessesTags) {
    Osmium::OSM::TagList taglist;

    taglist.add("entry1", "value1");
    taglist.add("entry2", "value2");

    BOOST_CHECK_EQUAL(taglist[0].key(), "entry1");
    BOOST_CHECK_EQUAL(taglist[1].key(), "entry2");
}

BOOST_AUTO_TEST_CASE(TagList_constBracketOperator_accessesTags) {
    Osmium::OSM::TagList taglist;

    taglist.add("entry1", "value1");
    taglist.add("entry2", "value2");

    const Osmium::OSM::TagList const_taglist = taglist;

    BOOST_CHECK_EQUAL(const_taglist[0].key(), "entry1");
    BOOST_CHECK_EQUAL(const_taglist[1].key(), "entry2");
}

BOOST_AUTO_TEST_CASE(TagList_iterator_accesesTags) {
    Osmium::OSM::TagList taglist;

    taglist.add("entry1", "value1");
    taglist.add("entry2", "value2");

    Osmium::OSM::TagList::iterator it = taglist.begin();
    BOOST_CHECK_EQUAL(it->key(), "entry1");

    it++;
    BOOST_CHECK_EQUAL(it->key(), "entry2");

    it++;
    BOOST_CHECK_EQUAL(it == taglist.end(), true);
}

BOOST_AUTO_TEST_CASE(TagList_constIterator_accesesTags) {
    Osmium::OSM::TagList taglist;

    taglist.add("entry1", "value1");
    taglist.add("entry2", "value2");

    const Osmium::OSM::TagList const_taglist = taglist;
    Osmium::OSM::TagList::const_iterator it = const_taglist.begin();
    BOOST_CHECK_EQUAL(it->key(), "entry1");

    it++;
    BOOST_CHECK_EQUAL(it->key(), "entry2");

    it++;
    BOOST_CHECK_EQUAL(it == const_taglist.end(), true);
}

BOOST_AUTO_TEST_CASE(TagList_getValueByKey_returnsRequestedValue) {
    Osmium::OSM::TagList taglist;

    taglist.add("entry1", "value1");
    taglist.add("entry2", "value2");

    BOOST_CHECK_EQUAL(taglist.get_value_by_key("entry1"), "value1");
    BOOST_CHECK_EQUAL(taglist.get_value_by_key("entry2"), "value2");
}

BOOST_AUTO_TEST_CASE(TagList_getValueByKey_returns0IfKeyIsNotPresent) {
    Osmium::OSM::TagList taglist;

    taglist.add("entry1", "value1");
    taglist.add("entry2", "value2");

    BOOST_CHECK_EQUAL((uintptr_t)taglist.get_value_by_key("something_else"), 0);
}

BOOST_AUTO_TEST_SUITE_END()
