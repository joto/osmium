#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/ser/item.hpp>

BOOST_AUTO_TEST_SUITE(SerObject)

BOOST_AUTO_TEST_CASE(ser_object) {
    Osmium::Ser::Object o;

    o.id(17);
    o.version(3);
    o.visible(true);
    o.changeset(333);
    o.uid(21);
    o.timestamp(123);

    BOOST_CHECK_EQUAL(17, o.id());
    BOOST_CHECK_EQUAL(3, o.version());
    BOOST_CHECK_EQUAL(true, o.visible());
    BOOST_CHECK_EQUAL(333, o.changeset());
    BOOST_CHECK_EQUAL(21, o.uid());
    BOOST_CHECK_EQUAL(123, o.timestamp());

    o.visible(false);
    BOOST_CHECK_EQUAL(3, o.version());
    BOOST_CHECK_EQUAL(false, o.visible());

    o.version(100);
    BOOST_CHECK_EQUAL(100, o.version());
    BOOST_CHECK_EQUAL(false, o.visible());
}

BOOST_AUTO_TEST_CASE(ser_object_string) {
    Osmium::Ser::Object o;

    o.id("17");
    o.version("3");
    o.visible(true);
    o.changeset("333");
    o.uid("21");

    BOOST_CHECK_EQUAL(17, o.id());
    BOOST_CHECK_EQUAL(3, o.version());
    BOOST_CHECK_EQUAL(true, o.visible());
    BOOST_CHECK_EQUAL(333, o.changeset());
    BOOST_CHECK_EQUAL(21, o.uid());
}

BOOST_AUTO_TEST_SUITE_END()
