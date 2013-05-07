#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <osmium/osmfile.hpp>

BOOST_AUTO_TEST_SUITE(OSMFile)

BOOST_AUTO_TEST_CASE(filename_osm) {
    Osmium::OSMFile file("test.osm");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XML());
}

BOOST_AUTO_TEST_CASE(filename_osm_bz2) {
    Osmium::OSMFile file("test.osm.bz2");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XMLbz2());
}

BOOST_AUTO_TEST_CASE(filename_osm_gz) {
    Osmium::OSMFile file("test.osm.gz");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XMLgz());
}

BOOST_AUTO_TEST_CASE(filename_osm_pbf) {
    Osmium::OSMFile file("test.osm.pbf");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());
}

BOOST_AUTO_TEST_CASE(filename_pbf) {
    Osmium::OSMFile file("test.pbf");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());
}

BOOST_AUTO_TEST_CASE(filename_osh_pbf) {
    Osmium::OSMFile file("test.osh.pbf");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::History());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());
}

BOOST_AUTO_TEST_CASE(filename_with_dir) {
    Osmium::OSMFile file("somedir/test.osm");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XML());
}

BOOST_AUTO_TEST_CASE(filename_with_parent_dir) {
    Osmium::OSMFile file("../test.osm");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XML());
}

BOOST_AUTO_TEST_CASE(filename_no_suffix) {
    Osmium::OSMFile file("test");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());
}

BOOST_AUTO_TEST_CASE(filename_unknown_suffix) {
    Osmium::OSMFile file("test.test");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());
}

BOOST_AUTO_TEST_CASE(filename_empty) {
    Osmium::OSMFile file("");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());
}

BOOST_AUTO_TEST_CASE(filename_minus) {
    Osmium::OSMFile file("-");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());
}

BOOST_AUTO_TEST_SUITE_END()

