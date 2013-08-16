#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/filesystem.hpp>

#include <osmium/osmfile.hpp>

// these tests work only if your boost library is new enough to have
// boost_filesystem version 3
// if that is not the case we disable the test and add a dummy test
#if BOOST_FILESYSTEM_VERSION == 3

std::string example_file_content("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\n");

/* Test scenarios for OSMFile objects
 */

#include <temp_file_fixture.hpp>

// Disable the Boost.Test handling of SIGCHLD signals.
#if defined(BOOST_POSIX_API)
# define DISABLE_SIGCHLD() signal(SIGCHLD, SIG_IGN)
#else
# define DISABLE_SIGCHLD()
#endif

BOOST_AUTO_TEST_SUITE(OSMFile_Output)


/* Helper function
 * Verify if the istream <inputfile> contains exactly the text <expected_content>
 */
void compare_file_content(std::istream* inputfile, std::string& expected_content)
{
    std::string file_content((std::istreambuf_iterator<char>(*inputfile)),(std::istreambuf_iterator<char>()));

    BOOST_CHECK_EQUAL(file_content, expected_content);
}

/* Test basic file operations:
 * Open an output file and check if correct fd ist returned
 */
BOOST_AUTO_TEST_CASE( write_to_xml_output_file ) {
    TempFileFixture test_osm("test.osm");

    Osmium::OSMFile file(test_osm);
    BOOST_REQUIRE_EQUAL(file.fd(), -1);

    file.open_for_output();
    BOOST_REQUIRE_GE(file.fd(), 0);

    write(file.fd(), example_file_content.c_str(), example_file_content.size());
    file.close();
    BOOST_REQUIRE_EQUAL(file.fd(), -1);

    std::ifstream in(test_osm, std::ios::binary);
    compare_file_content(&in, example_file_content);
}


/* Test gz encoding of output file:
 * Open output file with "osm.gz" extension 
 * and check if file written is gzip encoded
 */
BOOST_AUTO_TEST_CASE( write_to_xml_gz_output_file ) {
    TempFileFixture test_osm_gz("test.osm.gz");

    Osmium::OSMFile file(test_osm_gz);
    file.open_for_output();
    write(file.fd(), example_file_content.c_str(), example_file_content.size());
    file.close();

    std::ifstream inputfile(test_osm_gz, std::ios::binary);
    boost::iostreams::filtering_istream in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(inputfile);
    compare_file_content(&in, example_file_content);
}


/* Test bzip encoding of output file:
 * Open output file with "osm.bz2" extension
 * and check if file written is bzip2 encoded
 */
BOOST_AUTO_TEST_CASE( write_to_xml_bz2_output_file ) {
    TempFileFixture test_osm_bz2("test.osm.bz2");

    Osmium::OSMFile file(test_osm_bz2);
    file.open_for_output();
    write(file.fd(), example_file_content.c_str(), example_file_content.size());
    file.close();

    std::ifstream inputfile(test_osm_bz2, std::ios::binary);
    boost::iostreams::filtering_istream in;
    in.push(boost::iostreams::bzip2_decompressor());
    in.push(inputfile);

    compare_file_content(&in, example_file_content);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(OSMFile_Input)

void read_from_fd_and_compare(int fd, std::string& expected_content) {
    const int buf_length = 1000;
    char buffer[buf_length];

    int read_length = read(fd, buffer, buf_length-1);

    BOOST_CHECK_EQUAL(std::string(buffer, read_length), expected_content);
}

/* Test basic file input operations:
 * Open an input file and check if correct content ist returned
 */
BOOST_AUTO_TEST_CASE( read_from_xml_file ) {
    TempFileFixture test_osm("test.osm");

    // write content
    std::ofstream outputfile(test_osm, std::ios::binary);
    outputfile << example_file_content;
    outputfile.close();

    Osmium::OSMFile file(test_osm);
    file.open_for_input();

    read_from_fd_and_compare(file.fd(), example_file_content);
    file.close();
}

/* Test gzip decoding of input file:
 * Write gzip compressed data
 * and read it back through OSMFile
 */
BOOST_AUTO_TEST_CASE( read_from_xml_gz_file ) {
    TempFileFixture test_osm_gz("test.osm.gz");

    // write content
    std::ofstream outputfile(test_osm_gz, std::ios::binary);
    boost::iostreams::filtering_ostream out;
    out.push(boost::iostreams::gzip_compressor());
    out.push(outputfile);
    out << example_file_content;
    boost::iostreams::close(out);

    Osmium::OSMFile file(test_osm_gz);
    file.open_for_input();
    read_from_fd_and_compare(file.fd(), example_file_content);
    file.close();
}

/* Test bzip2 decoding of input file:
 * Write bzip2 compressed data
 * and read it back through OSMFile
 */
BOOST_AUTO_TEST_CASE( read_from_xml_bz2_file ) {
    TempFileFixture test_osm_bz2("test.osm.bz2");
    // write content
    std::ofstream outputfile(test_osm_bz2, std::ios::binary);
    boost::iostreams::filtering_ostream out;
    out.push(boost::iostreams::bzip2_compressor());
    out.push(outputfile);
    out << example_file_content;
    boost::iostreams::close(out);

    Osmium::OSMFile file(test_osm_bz2);
    file.open_for_input();
    read_from_fd_and_compare(file.fd(), example_file_content);
    file.close();
}


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(OSMFile_Errors)

BOOST_AUTO_TEST_CASE( OSMFile_writingToReadonlyDirectory_shouldRaiseIOException ) {
    TempDirFixture ro_dir("ro_dir");
    ro_dir.create_ro();

    Osmium::OSMFile file((ro_dir.path / "test.osm").c_str());
    try {
        file.open_for_output();
        BOOST_ERROR( "open_for_output didn't raise IOError" );
    } catch ( Osmium::OSMFile::IOError const& ex ) {
        BOOST_CHECK_EQUAL( ex.filename(), (ro_dir.path / "test.osm").native() );
        BOOST_CHECK_EQUAL( ex.system_errno(), 13 );  // errno 13: Permission denied
    }
}

/* the following test case generates memory leaks, so it is commented out for the moment
 */
//BOOST_AUTO_TEST_CASE( OSMFile_writingToReadonlyDirectoryWithGzip_shouldRaiseIOException ) {
//    DISABLE_SIGCHLD();
//    TempDirFixture ro_dir("ro_dir");
//    ro_dir.create_ro();
//
//    Osmium::OSMFile file((ro_dir.path / "test.osm.gz").c_str());
//    file.open_for_output();
//    try {
//        file.close();
//        BOOST_ERROR( "file.close() didn't raise IOError" );
//    } catch ( Osmium::OSMFile::IOError const& ex ) {
//        BOOST_CHECK_EQUAL( ex.filename(), (ro_dir.path / "test.osm.gz").native() );
//        BOOST_CHECK_EQUAL( ex.system_errno(), 0 ); // subprocess error has no valid errno code
//    }
//}

BOOST_AUTO_TEST_CASE( OSMFile_readingNonexistingFile_shouldRaiseException ) {
    TempFileFixture nonexisting_osm("nonexisting.osm");
    Osmium::OSMFile file(nonexisting_osm);

    try {
        file.open_for_input();
        BOOST_ERROR( "open_for_input didn't raise IOError" );
    } catch ( Osmium::OSMFile::IOError const& ex ) {
        BOOST_CHECK_EQUAL( ex.filename(), (std::string&)nonexisting_osm );
        BOOST_CHECK_EQUAL( ex.system_errno(), 2 );  // errno 2: No such file or directory
    }

}

BOOST_AUTO_TEST_CASE( OSMFile_readingNonexistingFileWithGzip_shouldRaiseIOException ) {
    DISABLE_SIGCHLD();
    TempFileFixture nonexisting_osm_gz("nonexisting.osm.gz");
    Osmium::OSMFile file(nonexisting_osm_gz);

    file.open_for_input();
    try {
        file.close();
    } catch ( Osmium::OSMFile::IOError const& ex ) {
        BOOST_CHECK_EQUAL( ex.filename(), (std::string&)nonexisting_osm_gz );
        BOOST_CHECK_EQUAL( ex.system_errno(), 0 );  // subprocess error has no valid errno code
    }
}

BOOST_AUTO_TEST_SUITE_END()

#else
BOOST_AUTO_TEST_SUITE(Dummy)
BOOST_AUTO_TEST_CASE(DummyTest) {
}
BOOST_AUTO_TEST_SUITE_END()
#endif

