#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <osmium/osm/position.hpp>

BOOST_AUTO_TEST_SUITE(Position)

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    Osmium::OSM::Position p;
    BOOST_CHECK(!p.defined());
}

BOOST_AUTO_TEST_CASE(instantiation_with_double_parameters) {
    Osmium::OSM::Position p1(1.2, 4.5);
    BOOST_CHECK(p1.defined());
    BOOST_CHECK_EQUAL(p1.x(), 12000000);
    BOOST_CHECK_EQUAL(p1.y(), 45000000);
    BOOST_CHECK_EQUAL(p1.lon(), 1.2);
    BOOST_CHECK_EQUAL(p1.lat(), 4.5);
    Osmium::OSM::Position p2(p1);
    BOOST_CHECK_EQUAL(p2.lat(), 4.5);
    Osmium::OSM::Position p3 = p1;
    BOOST_CHECK_EQUAL(p3.lat(), 4.5);
}

BOOST_AUTO_TEST_CASE(equality) {
    Osmium::OSM::Position p1(1.2, 4.5);
    Osmium::OSM::Position p2(1.2, 4.5);
    Osmium::OSM::Position p3(1.5, 1.5);
    BOOST_CHECK_EQUAL(p1, p2);
    BOOST_CHECK(p1 != p3);
}

BOOST_AUTO_TEST_CASE(output) {
    Osmium::OSM::Position p(-3.2, 47.3);
    output_test_stream out;
    out << p;
    BOOST_CHECK(out.is_equal("(-3.2,47.3)"));
}

BOOST_AUTO_TEST_CASE(Position_constructor_initializesFrom64int) {
    int64_t x=12000000, y=45000000;
    Osmium::OSM::Position position(x,y);

    BOOST_CHECK_EQUAL(position.x(), 12000000);
    BOOST_CHECK_EQUAL(position.y(), 45000000);
}

BOOST_AUTO_TEST_CASE(Position_comparisonOperator_comparesFirstByxThenByyCoordinate) {
    Osmium::OSM::Position p1, p2;

    p1.x(12000000);
    p1.y(45000000);
    p2.x(12000000);
    p2.y(45000000);
    BOOST_CHECK_EQUAL(p1 < p2, false);
    BOOST_CHECK_EQUAL(p1 > p2, false);

    p1.x(12000000);
    p1.y(45000000);
    p2.x(13000000);
    p2.y(44000000);
    BOOST_CHECK_EQUAL(p1 < p2, true);
    BOOST_CHECK_EQUAL(p1 > p2, false);

    p1.x(12000000);
    p1.y(45000000);
    p2.x(12000000);
    p2.y(44000000);
    BOOST_CHECK_EQUAL(p1 < p2, false);
    BOOST_CHECK_EQUAL(p1 > p2, true);
}

BOOST_AUTO_TEST_SUITE_END()

