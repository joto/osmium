#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <vector>

#include <osmium/osm/tag_ostream.hpp>
#include <osmium/tags/to_string.hpp>
#include <osmium/utils/filter_and_accumulate.hpp>
#include <osmium/tags/key_filter.hpp>

BOOST_AUTO_TEST_SUITE(TagToString)

BOOST_AUTO_TEST_CASE(tag_to_string) {
    Osmium::OSM::Tag t1("highway", "primary");
    Osmium::OSM::Tag t2("name", "Main Street");

    {
        std::string out;
        Osmium::Tags::TagToStringOp op("", "PREFIX", "INFIX", "SUFFIX", "JOIN");
        op(out, t1);
        op(out, t2);
        BOOST_CHECK_EQUAL("PREFIXhighwayINFIXprimarySUFFIXJOINPREFIXnameINFIXMain StreetSUFFIX", out);
    }

    {
        std::string out;
        Osmium::Tags::TagToKeyEqualsValueStringOp op(",");
        op(out, t1);
        op(out, t2);
        BOOST_CHECK_EQUAL("highway=primary,name=Main Street", out);
    }

    {
        std::string out;
        Osmium::Tags::TagToHStoreStringOp op;
        op(out, t1);
        op(out, t2);
        BOOST_CHECK_EQUAL("\"highway\"=>\"primary\",\"name\"=>\"Main Street\"", out);
    }

}

BOOST_AUTO_TEST_CASE(escape) {
    std::string out;
    Osmium::OSM::Tag t1("name", "O'Rourke Street (\"Fool's Corner\")");

    Osmium::Tags::TagToHStoreStringOp op;
    op(out, t1);
    BOOST_CHECK_EQUAL("\"name\"=>\"O'Rourke Street (\\\"Fool's Corner\\\")\"", out);
}

BOOST_AUTO_TEST_CASE(filter_and_accumulate) {
    Osmium::OSM::Tag t1("highway", "primary");
    Osmium::OSM::Tag t2("name", "Main Street");

    Osmium::Tags::TagToKeyEqualsValueStringOp op(",");

    std::vector<Osmium::OSM::Tag> tags;
    tags.push_back(t1);
    tags.push_back(t2);

    Osmium::Tags::KeyFilter filter(true);
    filter.add(true, "source");
    filter.add(true, "odbl");

    std::string out = Osmium::filter_and_accumulate(tags, filter, std::string(), op);
    BOOST_CHECK_EQUAL("highway=primary,name=Main Street", out);
}

BOOST_AUTO_TEST_SUITE_END()

