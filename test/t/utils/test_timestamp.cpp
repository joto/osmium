#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/utils/timestamp.hpp>

BOOST_AUTO_TEST_SUITE(Timestamp)

BOOST_AUTO_TEST_CASE(zero) {
    BOOST_CHECK_EQUAL(std::string(""), Osmium::Timestamp::to_iso(0));
}

BOOST_AUTO_TEST_CASE(second_after_epoch) {
    BOOST_CHECK_EQUAL(std::string("1970-01-01T00:00:01Z"), Osmium::Timestamp::to_iso(1));
}

BOOST_AUTO_TEST_CASE(sometime) {
    const char* ts= "2011-10-28T09:12:00Z";
    time_t t = Osmium::Timestamp::parse_iso(ts);
    BOOST_CHECK_EQUAL(std::string(ts), Osmium::Timestamp::to_iso(t));
}

BOOST_AUTO_TEST_SUITE_END()

