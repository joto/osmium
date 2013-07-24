#ifndef OSMIUM_OSMFILE_HPP
#define OSMIUM_OSMFILE_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <cerrno>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <boost/utility.hpp>

namespace Osmium {

    /**
     * This class describes an %OSM file in one of several different formats.
     * It can be used as factory class for generating input and output OSM files.
     *
     * If the filename is empty, this means stdin or stdout is used. If you set
     * the filename to "-" it will be treated the same.
     */
    class OSMFile {

    public:

        /**
         * Low-level system call error.
         * This should normally not happen unless your system is out of
         * resources like number of processes or filehandles.
         */
        class SystemError : public std::runtime_error {

            int m_errno;

        public:

            SystemError(const std::string& whatarg, int e) :
                std::runtime_error(whatarg),
                m_errno(e) {
            }

            /**
             * Get the system errno variable from the system call that caused
             * this exception.
             */
            int system_errno() const {
                return m_errno;
            }

        };

        /**
         * Low-level I/O Error.
         * This exception if thrown if there is an error in an I/O system call.
         */
        class IOError : public std::runtime_error {

            std::string m_filename;
            int m_errno;

        public:

            IOError(const std::string& whatarg,
                    const std::string& filename,
                    int e) :
                std::runtime_error(whatarg),
                m_filename(filename),
                m_errno(e) {
            }

            ~IOError() throw() {
            }

            /**
             * Get the filename that caused this exception.
             */
            const std::string& filename() const {
                return m_filename;
            }

            /**
             * Get the system errno variable from the system call that caused
             * this exception.
             */
            int system_errno() const {
                return m_errno;
            }

        };

        class ArgumentError : public std::runtime_error {

            std::string m_value;

        public:

            ArgumentError(const std::string& whatarg,
                          const std::string& value="") :
                std::runtime_error(whatarg),
                m_value(value) {
            }

            ~ArgumentError() throw() {
            }

            const std::string& value() const {
                return m_value;
            }

        };

        /**
         * An exception of a subclass of this class is thrown when the type of
         * a file is not what you expected.
         */
        struct FileTypeError {
        };

        /**
         * This exception is thrown when you wanted to read a normal OSM file,
         * but the file opened had a different type.
         */
        struct FileTypeOSMExpected : public FileTypeError {
        };

        /**
         * This exception is thrown when you wanted to read an OSM file with
         * historic information, but the file opened had a different type.
         */
        struct FileTypeHistoryExpected : public FileTypeError {
        };

        class FileEncodingNotSupported {
        };

        /**
         * Instances of this class describe different file types.
         *
         * You can not create instances of this class yourself, instead use
         * the static methods provided to get the predefined instances.
         */
        class FileType : boost::noncopyable {

            std::string m_suffix;
            bool m_has_multiple_object_versions;

            FileType(std::string suffix, bool has_multiple_object_versions) :
                m_suffix(suffix),
                m_has_multiple_object_versions(has_multiple_object_versions) {
            }

        public:

            std::string suffix() const {
                return m_suffix;
            }

            bool has_multiple_object_versions() const {
                return m_has_multiple_object_versions;
            }

            /**
             * Normal OSM file without history.
             */
            static FileType* OSM() {
                static FileType instance(".osm", false);
                return &instance;
            }

            /**
             * OSM file with history.
             */
            static FileType* History() {
                static FileType instance(".osh", true);
                return &instance;
            }

            /**
             * OSM change file.
             */
            static FileType* Change() {
                static FileType instance(".osc", true);
                return &instance;
            }

        };

        /**
         * Instances of this class describe different file encodings (ie PBF,
         * XML or different compressed versions of XML).
         *
         * You can not create instances of this class yourself, instead use
         * the static methods provided to get the predefined instances.
         */
        class FileEncoding : boost::noncopyable {

            const std::string m_suffix;
            const std::string m_compress;
            const std::string m_decompress;
            const bool m_pbf;

            FileEncoding(const std::string& suffix, const std::string& compress, const std::string& decompress, bool pbf) :
                m_suffix(suffix),
                m_compress(compress),
                m_decompress(decompress),
                m_pbf(pbf) {
            }

        public:

            const std::string& suffix() const {
                return m_suffix;
            }

            const std::string& compress() const {
                return m_compress;
            }

            const std::string& decompress() const {
                return m_decompress;
            }

            bool is_pbf() const {
                return m_pbf;
            }

            /**
             * Encoding in PBF.
             */
            static FileEncoding* PBF() {
                static FileEncoding instance(".pbf", "", "", true);
                return &instance;
            }

            /**
             * XML encoding, uncompressed.
             */
            static FileEncoding* XML() {
                static FileEncoding instance("", "", "", false);
                return &instance;
            }

            /**
             * XML encoding, compressed with gzip.
             */
            static FileEncoding* XMLgz() {
                static FileEncoding instance(".gz", "gzip", "zcat", false);
                return &instance;
            }

            /**
             * XML encoding, compressed with bzip2.
             */
            static FileEncoding* XMLbz2() {
                static FileEncoding instance(".bz2", "bzip2", "bzcat", false);
                return &instance;
            }

        };

    private:

        /// Type of file.
        FileType* m_type;

        /// Encoding of file.
        FileEncoding* m_encoding;

        /// File name.
        std::string m_filename;

        /// File descriptor. -1 before the file is opened.
        int m_fd;

        /**
         * Contains the child process id if a child was
         * created to uncompress data or for getting a
         * URL.
         */
        pid_t m_childpid;

        /**
         * Fork and execute the given command in the child.
         * A pipe is created between the child and the parent.
         * The child writes to the pipe, the parent reads from it.
         * This function never returns in the child.
         *
         * @param command Command to execute in the child.
         * @param input 0 for reading from child, 1 for writing to child.
         * @return File descriptor of pipe in the parent.
         * @throws SystemError if a system call fails.
         */
        int execute(const std::string& command, int input) {
            int pipefd[2];
            if (pipe(pipefd) < 0) {
                throw SystemError("Can't create pipe", errno);
            }
            pid_t pid = fork();
            if (pid < 0) {
                throw SystemError("Can't fork", errno);
            }
            if (pid == 0) { // child
                // close all file descriptors except one end of the pipe
                for (int i=0; i < 32; ++i) {
                    if (i != pipefd[1-input]) {
                        ::close(i);
                    }
                }
                if (dup2(pipefd[1-input], 1-input) < 0) { // put end of pipe as stdout/stdin
                    exit(1);
                }

                if (input == 0) {
                    ::open("/dev/null", O_RDONLY); // stdin
                    ::open("/dev/null", O_WRONLY); // stderr
                    if (::execlp(command.c_str(), command.c_str(), m_filename.c_str(), NULL) < 0) {
                        exit(1);
                    }
                } else {
                    if (::open(m_filename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666) != 1) {
                        exit(1);
                    }
                    ::open("/dev/null", O_WRONLY); // stderr
                    if (::execlp(command.c_str(), command.c_str(), 0, NULL) < 0) {
                        exit(1);
                    }
                }
            }
            // parent
            m_childpid = pid;
            ::close(pipefd[1-input]);
            return pipefd[input];
        }

        /**
         * Open OSMFile for reading.
         *
         * @return File descriptor of open file.
         * @throws IOError if the file can't be opened.
         */
        int open_input_file() const {
            if (m_filename == "") {
                return 0; // stdin
            } else {
                int flags = O_RDONLY;
#ifdef WIN32
                flags |= O_BINARY;
#endif
                int fd = ::open(m_filename.c_str(), flags);
                if (fd < 0) {
                    throw IOError("Open failed", m_filename, errno);
                }
                return fd;
            }
        }

        /**
         * Open OSMFile for writing. If the file exists, it is truncated, if
         * not it is created.
         *
         * @return File descriptor of open file.
         * @throws IOError if the file can't be opened.
         */
        int open_output_file() const {
            if (m_filename == "") {
                return 1; // stdout
            } else {
                int flags = O_WRONLY | O_TRUNC | O_CREAT;
#ifdef WIN32
                flags |= O_BINARY;
#endif
                int fd = ::open(m_filename.c_str(), flags, 0666);
                if (fd < 0) {
                    throw IOError("Open failed", m_filename, errno);
                }
                return fd;
            }
        }

        /**
         * Open OSMFile for reading. Handles URLs or normal files. URLs
         * are opened by executing the "curl" program (which must be installed)
         * and reading from its output.
         *
         * @return File descriptor of open file or pipe.
         * @throws SystemError if a system call fails.
         * @throws IOError if the file can't be opened.
         */
        int open_input_file_or_url() {
            std::string protocol = m_filename.substr(0, m_filename.find_first_of(':'));
            if (protocol == "http" || protocol == "https") {
                return execute("curl", 0);
            } else {
                return open_input_file();
            }
        }

    public:

        /**
         * Create OSMFile using type and encoding from filename. If you want
         * to overwrite these settings you can change them later.
         *
         * @param filename Filename including suffix. The type and encoding
         *                 of the file will be taken from the suffix.
         *                 An empty filename or "-" means stdin or stdout.
         */
        OSMFile(const std::string& filename = "") :
            m_type(FileType::OSM()),
            m_encoding(FileEncoding::PBF()),
            m_filename(filename),
            m_fd(-1),
            m_childpid(0) {

            // stdin/stdout
            if (filename == "" || filename == "-") {
                m_filename = "";
                default_settings_for_stdinout();
                return;
            }

            // filename is actually a URL
            std::string protocol = m_filename.substr(0, m_filename.find_first_of(':'));
            if (protocol == "http" || protocol == "https") {
                default_settings_for_url();
                return;
            }

            // isolate filename suffix
            size_t n = filename.find_last_of('/');
            if (n == std::string::npos) {
                n = 0;
            } else {
                ++n;
            }
            std::string suffix(filename.substr(filename.find_first_of('.', n)+1));

            set_type_and_encoding(suffix);
        }

        void set_type_and_encoding(const std::string& suffix) {
            if (suffix == "pbf" || suffix == "osm.pbf") {
                m_type     = FileType::OSM();
                m_encoding = FileEncoding::PBF();
            } else if (suffix == "osm") {
                m_type     = FileType::OSM();
                m_encoding = FileEncoding::XML();
            } else if (suffix == "osm.bz2") {
                m_type     = FileType::OSM();
                m_encoding = FileEncoding::XMLbz2();
            } else if (suffix == "osm.gz") {
                m_type     = FileType::OSM();
                m_encoding = FileEncoding::XMLgz();
            } else if (suffix == "osh.pbf") {
                m_type     = FileType::History();
                m_encoding = FileEncoding::PBF();
            } else if (suffix == "osh") {
                m_type     = FileType::History();
                m_encoding = FileEncoding::XML();
            } else if (suffix == "osh.bz2") {
                m_type     = FileType::History();
                m_encoding = FileEncoding::XMLbz2();
            } else if (suffix == "osh.gz") {
                m_type     = FileType::History();
                m_encoding = FileEncoding::XMLgz();
            } else if (suffix == "osc") {
                m_type     = FileType::Change();
                m_encoding = FileEncoding::XML();
            } else if (suffix == "osc.bz2") {
                m_type     = FileType::Change();
                m_encoding = FileEncoding::XMLbz2();
            } else if (suffix == "osc.gz") {
                m_type     = FileType::Change();
                m_encoding = FileEncoding::XMLgz();
            } else {
                default_settings_for_file();
            }
        }

        /**
         * Copy constructor.
         * Only attributes not related to the open file will be
         * copied.
         */
        OSMFile(const OSMFile& orig) :
            m_type(orig.type()),
            m_encoding(orig.encoding()),
            m_filename(orig.filename()),
            m_fd(-1),
            m_childpid(0) {
        }

        /**
         * Assignement operator.
         * Only attributes not related to the open file will be
         * copied.
         */
        OSMFile& operator=(const OSMFile& orig) {
            m_fd       = -1;
            m_childpid = 0;
            m_type     = orig.type();
            m_encoding = orig.encoding();
            m_filename = orig.filename();
            return *this;
        }

        ~OSMFile() {
            try {
                close();
            } catch (...) {
                // ignore exceptions
            }
        }

        void close() {
            if (m_fd > 0) {
                ::close(m_fd);
                m_fd = -1;
            }

            if (m_childpid) {
                int status;
                pid_t pid = waitpid(m_childpid, &status, 0);
                if (pid < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                    // global variable errno doesn't contain a valid value,
                    // so we better set errno=0
                    throw IOError("Subprocess returned error", m_filename, 0);
                }
                m_childpid = 0;
            }
        }

        /**
         * Set default settings for type and encoding when the filename is
         * empty or "-". If you want to have a different default setting
         * override this in a subclass.
         */
        void default_settings_for_stdinout() {
            m_type     = FileType::OSM();
            m_encoding = FileEncoding::PBF();
        }

        /**
         * Set default settings for type and encoding when the filename is
         * a normal file. If you want to have a different default setting
         * override this in a subclass.
         */
        void default_settings_for_file() {
            m_type     = FileType::OSM();
            m_encoding = FileEncoding::PBF();
        }

        /**
         * Set default settings for type and encoding when the filename is a URL.
         * If you want to have a different default setting override this in a
         * subclass.
         */
        void default_settings_for_url() {
            m_type     = FileType::OSM();
            m_encoding = FileEncoding::XML();
        }

        int fd() const {
            return m_fd;
        }

        FileType* type() const {
            return m_type;
        }

        OSMFile& type(FileType* type) {
            m_type = type;
            return *this;
        }

        OSMFile& type(const std::string& type) {
            if (type == "osm") {
                m_type = FileType::OSM();
            } else if (type == "history" || type == "osh") {
                m_type = FileType::History();
            } else if (type == "change" || type == "osc") {
                m_type = FileType::Change();
            } else {
                throw ArgumentError("Unknown OSM file type", type);
            }
            return *this;
        }

        bool has_multiple_object_versions() const {
            return m_type->has_multiple_object_versions();
        }

        FileEncoding* encoding() const {
            return m_encoding;
        }

        OSMFile& encoding(FileEncoding* encoding) {
            m_encoding = encoding;
            return *this;
        }

        OSMFile& encoding(const std::string& encoding) {
            if (encoding == "pbf") {
                m_encoding = FileEncoding::PBF();
            } else if (encoding == "xml") {
                m_encoding = FileEncoding::XML();
            } else if (encoding == "xmlgz" || encoding == "gz") {
                m_encoding = FileEncoding::XMLgz();
            } else if (encoding == "xmlbz2" || encoding == "bz2") {
                m_encoding = FileEncoding::XMLbz2();
            } else {
                throw ArgumentError("Unknown OSM file encoding", encoding);
            }
            return *this;
        }

        OSMFile& filename(const std::string& filename) {
            if (filename == "-") {
                m_filename = "";
            } else {
                m_filename = filename;
            }
            return *this;
        }

        const std::string& filename() const {
            return m_filename;
        }

        std::string filename_without_suffix() const {
            return m_filename.substr(0, m_filename.find_first_of('.'));
        }

        std::string filename_with_default_suffix() const {
            std::string filename = filename_without_suffix();
            filename += m_type->suffix() + m_encoding->suffix();
            return filename;
        }

        void open_for_input() {
            m_fd = m_encoding->decompress() == "" ? open_input_file_or_url() : execute(m_encoding->decompress(), 0);
        }

        void open_for_output() {
            m_fd = m_encoding->compress() == "" ? open_output_file() : execute(m_encoding->compress(), 1);
        }

    }; // class OSMFile

} // namespace Osmium

#endif // OSMIUM_OSMFILE_HPP
