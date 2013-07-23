#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include<osmium/osmfile.hpp>

BOOST_AUTO_TEST_SUITE(OSMFile_URL_handling)

BOOST_AUTO_TEST_CASE( OSMFile_ifInstantiatedWithURL_hasSaneDefaults ) {
    Osmium::OSMFile file("http://example.com/test.php");

    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XML());
}

BOOST_AUTO_TEST_SUITE_END()
