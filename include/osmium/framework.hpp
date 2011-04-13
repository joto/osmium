#ifndef OSMIUM_FRAMEWORK_HPP
#define OSMIUM_FRAMEWORK_HPP

#include <fcntl.h>

#ifdef OSMIUM_WITH_GEOS
# include <geos/geom/GeometryFactory.h>
# include <geos/geom/PrecisionModel.h>
#endif // OSMIUM_WITH_GEOS

namespace Osmium {

    class Framework {

    public:

        Framework(bool debug = false) {
            if (Osmium::global.framework) {
                throw std::runtime_error("Do not instantiate Osmium::Framework more than once!");
            }
            Osmium::global.debug = debug;
#ifdef OSMIUM_WITH_GEOS
            geos::geom::PrecisionModel pm;
            Osmium::global.geos_geometry_factory = new geos::geom::GeometryFactory(&pm, -1);
#endif // OSMIUM_WITH_GEOS
            Osmium::global.framework = this;
        }

        ~Framework() {
            Osmium::Input::PBF<Osmium::Handler::Base>::cleanup();
#ifdef OSMIUM_WITH_GEOS
            delete Osmium::global.geos_geometry_factory;
#endif // OSMIUM_WITH_GEOS
        }

        /**
        *  Parse OSM file and call callback functions.
        *  This works for OSM XML files (suffix .osm) and OSM binary files (suffix .pbf).
        *  Reads from STDIN if the filename is '-', in this case it assumes XML format.
        */
        template <class T>
        void parse_osmfile(char *osmfilename, T* handler = NULL) const {
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
