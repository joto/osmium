#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/tag_ostream.hpp>
#include <osmium/tags/regex_filter.hpp>

BOOST_AUTO_TEST_SUITE(RegexFilter)

BOOST_AUTO_TEST_CASE(regex_filter) {
    Osmium::Tags::RegexFilter filter(false);
    filter.add(true, "^highway$", "^(motorway|trunk|primary)(_link)?$");
    filter.add(true, "^highway$", "^residential$");
    filter.add(true, "^oneway$");

    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "primary")));
    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "primary_link")));
    BOOST_CHECK(!filter(Osmium::OSM::Tag("blurb", "flurb")));
    BOOST_CHECK(filter(Osmium::OSM::Tag("highway", "residential")));
    BOOST_CHECK(filter(Osmium::OSM::Tag("oneway", "yes")));

    {
        Osmium::OSM::TagList tags;
        tags.add("highway", "residential");
        tags.add("name", "Main Street");

        Osmium::Tags::RegexFilter::iterator fi_begin(filter, tags.begin(), tags.end());
        Osmium::Tags::RegexFilter::iterator fi_end(filter, tags.end(), tags.end());

        BOOST_CHECK(fi_begin != fi_end);
        BOOST_CHECK_EQUAL(Osmium::OSM::Tag("highway", "residential"), *fi_begin++);
        BOOST_CHECK(fi_begin == fi_end);
    }
}

BOOST_AUTO_TEST_SUITE_END()

