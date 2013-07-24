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


BOOST_AUTO_TEST_CASE(filename_osh) {
    Osmium::OSMFile file("test.osh");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::History());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XML());
}

BOOST_AUTO_TEST_CASE(filename_osh_gz) {
    Osmium::OSMFile file("test.osh.gz");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::History());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XMLgz());
}

BOOST_AUTO_TEST_CASE(filename_osh_bz2) {
    Osmium::OSMFile file("test.osh.bz2");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::History());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XMLbz2());
}

BOOST_AUTO_TEST_CASE(filename_osc) {
    Osmium::OSMFile file("test.osc");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::Change());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XML());
}

BOOST_AUTO_TEST_CASE(filename_osc_gz) {
    Osmium::OSMFile file("test.osc.gz");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::Change());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XMLgz());
}

BOOST_AUTO_TEST_CASE(filename_osc_bz2) {
    Osmium::OSMFile file("test.osc.bz2");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::Change());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XMLbz2());
}

BOOST_AUTO_TEST_CASE(OSMFile_type_shouldChangeFileType) {
    Osmium::OSMFile file("test.osm");

    file.type(Osmium::OSMFile::FileType::Change());
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::Change());
}

BOOST_AUTO_TEST_CASE(OSMFile_typeCalledWithString_shouldChangeFileType) {
    Osmium::OSMFile file("test.osm");

    file.type("osm");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::OSM());

    file.type("history");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::History());
    file.type("osh");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::History());

    file.type("change");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::Change());
    file.type("osc");
    BOOST_CHECK_EQUAL(file.type(), Osmium::OSMFile::FileType::Change());

    try {
        file.type("invalid_type");
        BOOST_ERROR("file.type didn't throw ArgumentError");
    } catch (Osmium::OSMFile::ArgumentError const& ex) {
        BOOST_CHECK_EQUAL( ex.value(), "invalid_type");
    }
}

BOOST_AUTO_TEST_CASE(OSMFile_encoding_shouldChangeEncoding) {
    Osmium::OSMFile file("test.osm");

    file.encoding(Osmium::OSMFile::FileEncoding::PBF());
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());
}

BOOST_AUTO_TEST_CASE(OSMFile_encodingCalledWithString_shouldChangeEncoding) {
    Osmium::OSMFile file("test.osm");

    file.encoding("pbf");
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::PBF());

    file.encoding("xml");
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XML());

    file.encoding("xmlgz");
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XMLgz());

    file.encoding("xmlbz2");
    BOOST_CHECK_EQUAL(file.encoding(), Osmium::OSMFile::FileEncoding::XMLbz2());

    try {
        file.encoding("invalid_encoding");
        BOOST_ERROR("file.encoding didn't throw ArgumentError");
    } catch (Osmium::OSMFile::ArgumentError const& ex) {
        BOOST_CHECK_EQUAL(ex.value(), "invalid_encoding");
    }
}

BOOST_AUTO_TEST_CASE(OSMFile_filenameWithoutSuffix_shouldReturnFilenameWithoutSuffix) {
    Osmium::OSMFile file1("test.osm");
    BOOST_CHECK_EQUAL(file1.filename_without_suffix(), "test");

    Osmium::OSMFile file2("something_else.osm.bz2");
    BOOST_CHECK_EQUAL(file2.filename_without_suffix(), "something_else");
}

BOOST_AUTO_TEST_CASE(OSMFile_filenameWithDefaultSuffix_shouldReturnCorrectFilenames) {
    Osmium::OSMFile file("test.osm");
    BOOST_CHECK_EQUAL(file.filename_with_default_suffix(), "test.osm");

    file.encoding(Osmium::OSMFile::FileEncoding::PBF());
    BOOST_CHECK_EQUAL(file.filename_with_default_suffix(), "test.osm.pbf");

    file.encoding(Osmium::OSMFile::FileEncoding::XMLgz());
    BOOST_CHECK_EQUAL(file.filename_with_default_suffix(), "test.osm.gz");
}

BOOST_AUTO_TEST_SUITE_END()
