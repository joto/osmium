#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/node.hpp>

BOOST_AUTO_TEST_SUITE(Object)

void check_taglists_for_equality(const Osmium::OSM::TagList &tl1, const Osmium::OSM::TagList &tl2) {
    Osmium::OSM::TagList::const_iterator it1 = tl1.begin(), end1=tl1.end();
    Osmium::OSM::TagList::const_iterator it2 = tl2.begin(), end2=tl2.end();

    while ( (it1 != end1) && (it2 != end2) ) {
        BOOST_CHECK_EQUAL(it1->key(), it2->key());
        BOOST_CHECK_EQUAL(it1->value(), it2->value());
        ++it1;
        ++it2;
    }
    BOOST_CHECK( (it1 == end1) && (it2 == end2));
}

BOOST_AUTO_TEST_CASE(Object_id_shouldDecodeStrings) {
    Osmium::OSM::Node obj;

    obj.id("12");
    BOOST_CHECK_EQUAL(obj.id(), 12);

    obj.id("-3");
    BOOST_CHECK_EQUAL(obj.id(), -3);
}

BOOST_AUTO_TEST_CASE(Object_version_shouldDecodeStrings) {
    Osmium::OSM::Node obj;

    obj.version("2");
    BOOST_CHECK_EQUAL(obj.version(), 2u);

    obj.version("3445");
    BOOST_CHECK_EQUAL(obj.version(), 3445u);
}

BOOST_AUTO_TEST_CASE(Object_changeset_shouldDecodeStrings) {
    Osmium::OSM::Node obj;

    obj.changeset("2");
    BOOST_CHECK_EQUAL(obj.changeset(), 2);

    obj.changeset("3445");
    BOOST_CHECK_EQUAL(obj.changeset(), 3445);
}

BOOST_AUTO_TEST_CASE(Object_uid_shouldDecodeStrings) {
    Osmium::OSM::Node obj;

    obj.uid("2");
    BOOST_CHECK_EQUAL(obj.uid(), 2);

    obj.uid("3445");
    BOOST_CHECK_EQUAL(obj.uid(), 3445);
}

BOOST_AUTO_TEST_CASE(Object_userIsAnonymous_considersMinusOneAsAnonymouse) {
    Osmium::OSM::Node obj;

    obj.uid("-1");
    BOOST_CHECK_EQUAL(obj.user_is_anonymous(), true);

    obj.uid("37331");
    BOOST_CHECK_EQUAL(obj.user_is_anonymous(), false);
}

BOOST_AUTO_TEST_CASE(Object_timestampAsString_convertsTimestampToIsoFormat) {
    Osmium::OSM::Node obj;
    time_t ts = 1362135600u;

    obj.timestamp(ts);
    BOOST_CHECK_EQUAL(obj.timestamp_as_string(), "2013-03-01T11:00:00Z");
}


BOOST_AUTO_TEST_CASE(Object_timestampAsString_considersZeroAsUnset) {
    Osmium::OSM::Node obj;

    obj.timestamp((time_t)0u);
    BOOST_CHECK_EQUAL(obj.timestamp_as_string(), "");
}


BOOST_AUTO_TEST_CASE(Object_timestamp_convertsIsoFormatToTimestamp) {
    Osmium::OSM::Node obj;

    obj.timestamp("2013-03-01T11:00:00Z");
    BOOST_CHECK_EQUAL(obj.timestamp(), (time_t)1362135600u);
}

BOOST_AUTO_TEST_CASE(Object_timestampCalledWithInvalidFormat_throwsInvalidArgument) {
    Osmium::OSM::Node obj;

    BOOST_CHECK_THROW(obj.timestamp("invalid_timestamp"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Object_endtime_setsEndtime) {
    Osmium::OSM::Node obj;
    time_t ts = 1362135600u;

    obj.endtime(ts);
    BOOST_CHECK_EQUAL(obj.endtime(), ts);
}

BOOST_AUTO_TEST_CASE(Object_endtimeAsString_convertsTimestampToIsoFormat) {
    Osmium::OSM::Node obj;
    time_t ts = 1362135600u;

    obj.endtime(ts);
    BOOST_CHECK_EQUAL(obj.endtime_as_string(), "2013-03-01T11:00:00Z");
}


BOOST_AUTO_TEST_CASE(Object_endtimeAsString_considersZeroAsUnset) {
    Osmium::OSM::Node obj;

    obj.endtime((time_t)0u);
    BOOST_CHECK_EQUAL(obj.endtime_as_string(), "");
}


BOOST_AUTO_TEST_CASE(Object_changeset_setsChangeset) {
    Osmium::OSM::Node obj;

    obj.changeset(14);
    BOOST_CHECK_EQUAL(obj.changeset(), 14);
}

BOOST_AUTO_TEST_CASE(Object_uid_setsUid) {
    Osmium::OSM::Node obj;

    obj.uid(15);
    BOOST_CHECK_EQUAL(obj.uid(), 15);
}

BOOST_AUTO_TEST_CASE(Object_user_setsUsername) {
    Osmium::OSM::Node obj;

    obj.user("L33t User");
    BOOST_CHECK_EQUAL(obj.user(), "L33t User");
}


BOOST_AUTO_TEST_CASE(Object_visible_setsVisible) {
    Osmium::OSM::Node obj;

    obj.visible(false);
    BOOST_CHECK_EQUAL(obj.visible(), false);

    obj.visible(true);
    BOOST_CHECK_EQUAL(obj.visible(), true);
}


BOOST_AUTO_TEST_CASE(Object_userCalledWithTooLongName_shouldThrowLenghtError) {
    Osmium::OSM::Node obj;
    std::string username_too_long("\
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

    BOOST_CHECK_THROW(obj.user(username_too_long.c_str()), std::length_error);
}


BOOST_AUTO_TEST_CASE(Object_visible_shouldConvertStringValues) {
    Osmium::OSM::Node obj;

    obj.visible("true");
    BOOST_CHECK_EQUAL(obj.visible(), true);

    obj.visible("false");
    BOOST_CHECK_EQUAL(obj.visible(), false);

    obj.visible("arbitrary_string");
    BOOST_CHECK_EQUAL(obj.visible(), true);
}

BOOST_AUTO_TEST_CASE(Object_tags_setsTagList) {
    Osmium::OSM::Node obj;

    Osmium::OSM::TagList taglist;
    taglist.add("example", "one");

    obj.tags(taglist);
    check_taglists_for_equality(obj.tags(), taglist);
}

BOOST_AUTO_TEST_CASE(Object_setAttribute_shouldSetAttributesByName) {
    Osmium::OSM::Node obj;

    obj.set_attribute("id", "12");
    obj.set_attribute("version", "13");
    obj.set_attribute("changeset", "14");
    obj.set_attribute("timestamp", "2013-03-01T11:00:00Z");
    obj.set_attribute("uid", "15");
    obj.set_attribute("user", "L33t User");
    obj.set_attribute("visible", "false");

    BOOST_CHECK_EQUAL(obj.id(), 12);
    BOOST_CHECK_EQUAL(obj.version(), 13u);
    BOOST_CHECK_EQUAL(obj.changeset(), 14);
    BOOST_CHECK_EQUAL(obj.timestamp(), (time_t)1362135600u);
    BOOST_CHECK_EQUAL(obj.uid(), 15);
    BOOST_CHECK_EQUAL(obj.user(), "L33t User");
    BOOST_CHECK_EQUAL(obj.visible(), false);
}

BOOST_AUTO_TEST_CASE(Object_copyConstructor_copiesAllAttributes) {
    Osmium::OSM::Node src;

    src.id(12);
    src.version(13u);
    src.changeset(14);
    src.timestamp((time_t)1362135600u);
    src.uid(15);
    src.user("L33t User");
    src.visible(false);
    src.tags().add("example", "one");

    Osmium::OSM::Node dst(src);
    BOOST_CHECK_EQUAL(dst.id(), 12);
    BOOST_CHECK_EQUAL(dst.version(), 13u);
    BOOST_CHECK_EQUAL(dst.changeset(), 14);
    BOOST_CHECK_EQUAL(dst.timestamp(), (time_t)1362135600u);
    BOOST_CHECK_EQUAL(dst.uid(), 15);
    BOOST_CHECK_EQUAL(dst.user(), "L33t User");
    BOOST_CHECK_EQUAL(dst.visible(), false);
    BOOST_CHECK_EQUAL(dst.tags()[0].key(), "example");
    BOOST_CHECK_EQUAL(dst.tags()[0].value(), "one");
}

BOOST_AUTO_TEST_SUITE_END()
