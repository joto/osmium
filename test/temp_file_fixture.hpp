#ifndef TEMP_FILE_FIXTURE
#define TEMP_FILE_FIXTURE

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

boost::filesystem::path tempdir_path;

/* TempDirFixture:  Prepare a temp directory and clean up afterwards
 */
struct TempBaseDirFixture {
    TempBaseDirFixture() {
        tempdir_path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
        boost::filesystem::create_directory(tempdir_path);
    }

    ~TempBaseDirFixture() {
        boost::filesystem::remove_all(tempdir_path);
    }
};

struct TempDirFixture {
    TempDirFixture(const std::string& name) {
        path = tempdir_path / name;
    }

    ~TempDirFixture() {
        boost::filesystem::permissions(path, boost::filesystem::owner_all | boost::filesystem::add_perms);
        boost::filesystem::remove_all(path);
    }


    void create_ro() {
        boost::filesystem::create_directory(path);
        boost::filesystem::permissions(path, boost::filesystem::all_all | boost::filesystem::remove_perms);
    }

    operator const char*() const {
        return path.c_str();
    }

    operator const std::string&() const {
        return path.native();
    }

    boost::filesystem::path path;
};

struct TempFileFixture {
    TempFileFixture(const std::string& name) {
        path = tempdir_path / name;
    }

    ~TempFileFixture() {
        boost::filesystem::remove(path);
    }

    operator const char*() const {
        return path.c_str();
    }

    operator const std::string&() const {
        return path.native();
    }

    boost::filesystem::path path;
};


BOOST_GLOBAL_FIXTURE(TempBaseDirFixture)

#endif
