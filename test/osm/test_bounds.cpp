#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/bounds.hpp>

BOOST_AUTO_TEST_SUITE(Bounds)

BOOST_AUTO_TEST_CASE(instantiation) {
    Osmium::OSM::Bounds b;
    BOOST_CHECK(!b.defined());
    BOOST_CHECK(!b.bl().defined());
    BOOST_CHECK(!b.tr().defined());
}

BOOST_AUTO_TEST_CASE(instantiation_and_extend) {
    Osmium::OSM::Bounds b;
    b.extend(Osmium::OSM::Position(1.2, 3.4));
    BOOST_CHECK(b.defined());
    BOOST_CHECK(b.bl().defined());
    BOOST_CHECK(b.tr().defined());
    b.extend(Osmium::OSM::Position(3.4, 4.5));
    b.extend(Osmium::OSM::Position(5.6, 7.8));
    BOOST_CHECK_EQUAL(b.bl(), Osmium::OSM::Position(1.2, 3.4));
    BOOST_CHECK_EQUAL(b.tr(), Osmium::OSM::Position(5.6, 7.8));
}

BOOST_AUTO_TEST_CASE(output) {
    Osmium::OSM::Bounds b;
    b.extend(Osmium::OSM::Position(1.2, 3.4));
    b.extend(Osmium::OSM::Position(5.6, 7.8));
    std::ostringstream out;
    out << b;
    BOOST_CHECK_EQUAL(out.str(), "(1.2,3.4,5.6,7.8)");
}

BOOST_AUTO_TEST_SUITE_END()

