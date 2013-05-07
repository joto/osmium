#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <osmium/osm/bounds.hpp>

BOOST_AUTO_TEST_SUITE(Bounds)

BOOST_AUTO_TEST_CASE(instantiation) {
    Osmium::OSM::Bounds b;
    BOOST_CHECK(!b.defined());
    BOOST_CHECK(!b.bottom_left().defined());
    BOOST_CHECK(!b.top_right().defined());
}

BOOST_AUTO_TEST_CASE(instantiation_and_extend) {
    Osmium::OSM::Bounds b;
    b.extend(Osmium::OSM::Position(1.2, 3.4));
    BOOST_CHECK(b.defined());
    BOOST_CHECK(b.bottom_left().defined());
    BOOST_CHECK(b.top_right().defined());
    b.extend(Osmium::OSM::Position(3.4, 4.5));
    b.extend(Osmium::OSM::Position(5.6, 7.8));
    BOOST_CHECK_EQUAL(b.bottom_left(), Osmium::OSM::Position(1.2, 3.4));
    BOOST_CHECK_EQUAL(b.top_right(), Osmium::OSM::Position(5.6, 7.8));
}

BOOST_AUTO_TEST_CASE(output) {
    Osmium::OSM::Bounds b;
    b.extend(Osmium::OSM::Position(1.2, 3.4));
    b.extend(Osmium::OSM::Position(5.6, 7.8));
    output_test_stream out;
    out << b;
    BOOST_CHECK(out.is_equal("(1.2,3.4,5.6,7.8)"));
}

BOOST_AUTO_TEST_SUITE_END()

