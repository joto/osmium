#ifndef OSMIUM_OUTPUT_OSM_PBF_HPP
#define OSMIUM_OUTPUT_OSM_PBF_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <netinet/in.h>

namespace Osmium {

    namespace Output {

        namespace OSM {

            class PBF : public Base {

                int fd;

            protected:

                OSMPBF::Blob pbf_blob;

                void store_blob() {
                    std::string blob;
                    pbf_blob.SerializeToString(&blob);

                    OSMPBF::BlobHeader pbf_blob_header;
                    pbf_blob_header.set_datasize(blob.size());
                    pbf_blob_header.set_type("OSMHeader");

                    std::string blobhead;
                    pbf_blob_header.SerializeToString(&blobhead);

                    long int sz = htonl(blobhead.size());
                    ::write(fd, &sz, sizeof(sz));
                    ::write(fd, blobhead.c_str(), blobhead.size());
                    ::write(fd, blob.c_str(), blob.size());
                }

            public:

                PBF() : Base(), fd(1) {
                    GOOGLE_PROTOBUF_VERIFY_VERSION;
                }

                PBF(std::string &filename) : Base() {
                    GOOGLE_PROTOBUF_VERIFY_VERSION;

                    fd = open(filename.c_str(), O_WRONLY | O_TRUNC | O_CREAT);
                    if(-1 == fd)
                        perror("unable to open outfile");
                }

                static void cleanup() {
                    // this is needed even if the protobuf lib was never used so that valgrind doesn't report any errors
                    google::protobuf::ShutdownProtobufLibrary();
                }

                void write_init() {
                    pbf_blob.set_raw("foo raw data");
                    store_blob();
                }

                void write_bounds(double minlon, double minlat, double maxlon, double maxlat) {
                }

                void write(Osmium::OSM::Node *node) {
                }

                void write(Osmium::OSM::Way *way) {
                }

                void write(Osmium::OSM::Relation *relation) {
                }

                void write_final() {
                    if(fd > 0)
                        close(fd);
                }

            }; // class PBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBF_HPP
