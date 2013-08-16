#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <osmium/osm/tag_ostream.hpp>

BOOST_AUTO_TEST_SUITE(TagOstream)

BOOST_AUTO_TEST_CASE(Tag_serializedToOstream_givesTextualRepresentation) {
    boost::test_tools::output_test_stream output;
    Osmium::OSM::Tag tag("example", "value");

    output << tag;
    BOOST_CHECK(output.is_equal("example=value"));
}

BOOST_AUTO_TEST_SUITE_END()
