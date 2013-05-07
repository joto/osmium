#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/tag.hpp>
#include <osmium/osm/tag_ostream.hpp>

BOOST_AUTO_TEST_SUITE(Tag)

BOOST_AUTO_TEST_CASE(tag) {
    Osmium::OSM::Tag t1("foo", "bar");
    Osmium::OSM::Tag t2("foo", "bar");
    Osmium::OSM::Tag t3("foo", "baz");
    Osmium::OSM::Tag t4("x", "y");

    BOOST_CHECK_EQUAL(t1, t2);
    BOOST_CHECK(t1 != t3);
    BOOST_CHECK(t1 != t4);
    BOOST_CHECK(!strcmp("foo", t2.key()));
    BOOST_CHECK(!strcmp("bar", t2.value()));
}

BOOST_AUTO_TEST_SUITE_END()

