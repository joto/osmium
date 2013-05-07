#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/tag_ostream.hpp>
#include <osmium/tags/key_filter.hpp>
#include <osmium/tags/key_value_filter.hpp>

BOOST_AUTO_TEST_SUITE(Filter)

BOOST_AUTO_TEST_CASE(key_filter) {
    Osmium::Tags::KeyFilter filter(false);
    filter.add(true, "highway");
    filter.add(true, "name");

    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "primary")));
    BOOST_CHECK(!filter(Osmium::OSM::Tag("blurb", "flurb")));

    Osmium::OSM::TagList tags;
    tags.add("highway", "residential");
    tags.add("oneway", "yes");
    tags.add("name", "Main Street");

    Osmium::Tags::KeyFilter::iterator fi_begin(filter, tags.begin(), tags.end());
    Osmium::Tags::KeyFilter::iterator fi_end(filter, tags.end(), tags.end());

    BOOST_CHECK(fi_begin != fi_end);
    BOOST_CHECK_EQUAL(Osmium::OSM::Tag("highway", "residential"), *fi_begin++);
    BOOST_CHECK(fi_begin != fi_end);
    BOOST_CHECK_EQUAL(Osmium::OSM::Tag("name", "Main Street"), *fi_begin++);
    BOOST_CHECK(fi_begin == fi_end);
}

BOOST_AUTO_TEST_CASE(key_value_filter) {
    Osmium::Tags::KeyValueFilter filter(false);
    filter.add(true, "highway", "motorway");
    filter.add(true, "highway", "trunk");
    filter.add(true, "highway", "primary");

    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "primary")));
    BOOST_CHECK(!filter(Osmium::OSM::Tag("blurb", "flurb")));
    BOOST_CHECK(!filter(Osmium::OSM::Tag("highway", "residential")));

    {
        Osmium::OSM::TagList tags;
        tags.add("highway", "residential");
        tags.add("name", "Main Street");

        Osmium::Tags::KeyValueFilter::iterator fi_begin(filter, tags.begin(), tags.end());
        Osmium::Tags::KeyValueFilter::iterator fi_end(filter, tags.end(), tags.end());

        BOOST_CHECK(fi_begin == fi_end);
    }

    {
        Osmium::OSM::TagList tags;
        tags.add("highway", "primary");
        tags.add("name", "Main Street");

        Osmium::Tags::KeyValueFilter::iterator fi_begin(filter, tags.begin(), tags.end());
        Osmium::Tags::KeyValueFilter::iterator fi_end(filter, tags.end(), tags.end());

        BOOST_CHECK(fi_begin != fi_end);
        BOOST_CHECK_EQUAL(Osmium::OSM::Tag("highway", "primary"), *fi_begin++);
        BOOST_CHECK(fi_begin == fi_end);
    }
}

BOOST_AUTO_TEST_CASE(key_value_filter_empty) {
    Osmium::Tags::KeyValueFilter filter(false);
    filter.add(true, "highway", "");

    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "primary")));
    BOOST_CHECK(!filter(Osmium::OSM::Tag("blurb", "flurb")));
    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "residential")));

    Osmium::OSM::TagList tags;
    tags.add("highway", "primary");
    tags.add("name", "Main Street");

    Osmium::Tags::KeyValueFilter::iterator fi_begin(filter, tags.begin(), tags.end());
    Osmium::Tags::KeyValueFilter::iterator fi_end(filter, tags.end(), tags.end());

    BOOST_CHECK(fi_begin != fi_end);
    BOOST_CHECK_EQUAL(Osmium::OSM::Tag("highway", "primary"), *fi_begin++);
    BOOST_CHECK(fi_begin == fi_end);
}

BOOST_AUTO_TEST_CASE(key_value_filter_null) {
    Osmium::Tags::KeyValueFilter filter(false);
    filter.add(true, "highway");

    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "primary")));
    BOOST_CHECK(!filter(Osmium::OSM::Tag("blurb", "flurb")));
    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "residential")));
}

BOOST_AUTO_TEST_CASE(key_value_filter_tf) {
    Osmium::Tags::KeyValueFilter filter(false);
    filter.add(false, "highway", "residential");
    filter.add(true, "highway");

    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "primary")));
    BOOST_CHECK(!filter(Osmium::OSM::Tag("blurb", "flurb")));
    BOOST_CHECK(!filter(Osmium::OSM::Tag("highway", "residential")));
}

BOOST_AUTO_TEST_SUITE_END()

