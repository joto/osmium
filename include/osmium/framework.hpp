#ifndef OSMIUM_FRAMEWORK_HPP
#define OSMIUM_FRAMEWORK_HPP

#include <fcntl.h>

namespace Osmium {

    class Framework {

      public:

        bool debug;

        Framework() {
            debug = false;
        }

        ~Framework() {
            Osmium::Input::PBF<Osmium::Handler::Base>::cleanup();
        }

        /**
        *  Parse OSM file and call callback functions.
        *  This works for OSM XML files (suffix .osm) and OSM binary files (suffix .pbf).
        *  Reads from STDIN if the filename is '-', in this case it assumes XML format.
        */
        template <class T>
        void parse_osmfile(char *osmfilename, T* handler = NULL) {
            int fd = 0;
            if (osmfilename[0] == '-' && osmfilename[1] == '\0') {
                // fd is already 0, read STDIN
            } else {
                fd = open(osmfilename, O_RDONLY);
                if (fd < 0) {
                    std::cerr << "Can't open osm file: " << strerror(errno) << std::endl;
                    exit(1);
                }
            }

            Osmium::Input::Base<T> *input;

            char *suffix = strrchr(osmfilename, '.');
            if (suffix == NULL || !strcmp(suffix, ".osm")) {
                input = new Osmium::Input::XML<T>(fd, handler);
            } else if (!strcmp(suffix, ".pbf")) {
                input = new Osmium::Input::PBF<T>(fd, handler);
            } else {
                std::cerr << "Unknown file suffix: " << suffix << std::endl;
                exit(1);
            }

            input->parse();
            delete input;

            close(fd);
        }

    }; // class Framework

} // namespace Osmium

#endif // OSMIUM_FRAMEWORK_HPP
