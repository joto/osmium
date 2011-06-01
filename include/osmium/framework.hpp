#ifndef OSMIUM_FRAMEWORK_HPP
#define OSMIUM_FRAMEWORK_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

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
            // this is needed even if the protobuf lib was never used so that valgrind doesn't report any errors
            google::protobuf::ShutdownProtobufLibrary();
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
