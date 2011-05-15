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

                static const int NANO = 1000 * 1000 * 1000;
                static const int max_block_contents = 8000;
                FILE *fd;

                bool use_dense_format_;

                bool headerWritten;
                int block_contents_counter;

                OSMPBF::Blob pbf_blob;
                OSMPBF::BlobHeader pbf_blob_header;

                OSMPBF::HeaderBlock pbf_header_block;
                OSMPBF::PrimitiveBlock pbf_primitive_block;

                OSMPBF::PrimitiveGroup *pbf_primitive_group_nodes;
                OSMPBF::PrimitiveGroup *pbf_primitive_group_ways;
                OSMPBF::PrimitiveGroup *pbf_primitive_group_relations;

                void store_blob() {
                    std::string blob;
                    pbf_blob.SerializeToString(&blob);
                    pbf_blob.Clear();

                    pbf_blob_header.set_datasize(blob.size());

                    std::string blobhead;
                    pbf_blob_header.SerializeToString(&blobhead);
                    pbf_blob_header.Clear();

                    long int sz = htonl(blobhead.size());
                    fwrite(&sz, sizeof(sz), 1, fd);
                    fwrite(blobhead.c_str(), blobhead.size(), 1, fd);
                    fwrite(blob.c_str(), blob.size(), 1, fd);
                }

                void store_header_block() {
                    std::string header;
                    pbf_header_block.SerializeToString(&header);
                    pbf_header_block.Clear();

                    // TODO: add compression
                    pbf_blob_header.set_type("OSMHeader");
                    pbf_blob.set_raw(header);
                    store_blob();

                    headerWritten = true;
                }

                void store_primitive_block() {
                    if(block_contents_counter == 0)
                        return;

                    std::string block;
                    pbf_primitive_block.SerializeToString(&block);
                    pbf_primitive_block.Clear();

                    // add empty string-table entry at index 0
                    pbf_primitive_block.mutable_stringtable()->add_s("");

                    block_contents_counter = 0;

                    // TODO: add compression
                    pbf_blob_header.set_type("OSMData");
                    pbf_blob.set_raw(block);
                    store_blob();
                }

                void check_block_contents_counter() {
                    if(++block_contents_counter >= max_block_contents)
                        store_primitive_block();
                }

                unsigned int str2pbf(const std::string &str) {
                    // TODO: use a std::vector, avoid duplicates and sort before writing
                    OSMPBF::StringTable *st = pbf_primitive_block.mutable_stringtable();

                    // skip first slot
                    int i, l;
                    for(i = 1, l = st->s_size(); i<l; i++)
                        if(st->s(i) == str)
                            return i;

                    st->add_s(str);
                    return l;
                }

            public:

                PBF() : Base(),
                    fd(stdout),
                    use_dense_format_(true),
                    headerWritten(false),
                    block_contents_counter(0),
                    pbf_primitive_group_nodes(NULL),
                    pbf_primitive_group_ways(NULL),
                    pbf_primitive_group_relations(NULL) {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;
                }

                PBF(std::string &filename) : Base(),
                    use_dense_format_(true),
                    headerWritten(false),
                    block_contents_counter(0),
                    pbf_primitive_group_nodes(NULL),
                    pbf_primitive_group_ways(NULL),
                    pbf_primitive_group_relations(NULL) {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;

                    fd = fopen(filename.c_str(), "w");
                    if(!fd)
                        perror("unable to open outfile");
                }

                bool use_dense_format() const {
                    return use_dense_format_;
                }

                PBF& use_dense_format(bool d) {
                    use_dense_format_ = d;
                    return *this;
                }

                static void cleanup() {
                    // this is needed even if the protobuf lib was never used so that valgrind doesn't report any errors
                    google::protobuf::ShutdownProtobufLibrary();
                }

                void write_init() {
                    pbf_header_block.add_required_features("OsmSchema-V0.6");

                    if(use_dense_format())
                        pbf_header_block.add_required_features("DenseNodes");

                    if(is_history_file())
                        pbf_header_block.add_required_features("HistoricalInformation");

                    // add empty string-table entry at index 0
                    pbf_primitive_block.mutable_stringtable()->add_s("");
                    pbf_header_block.set_writingprogram("Osmium (http://wiki.openstreetmap.org/wiki/Osmium)");
                }

                void write_bounds(double minlon, double minlat, double maxlon, double maxlat) {
                    if(!headerWritten) {
                        // TODO: encode lat/lon
                        //pbf_header_block.bbox().set_left(minlon);
                        //pbf_header_block.bbox().set_top(minlat);
                        //pbf_header_block.bbox().set_right(maxlon);
                        //pbf_header_block.bbox().set_bottom(maxlat);
                    }
                }

                // TODO: use dense format if enabled
                void write(Osmium::OSM::Node *node) {
                    if(!headerWritten)
                        store_header_block();

                    if(!pbf_primitive_group_nodes)
                        pbf_primitive_group_nodes = pbf_primitive_block.add_primitivegroup();

                    OSMPBF::Node *pbf_node = pbf_primitive_group_nodes->add_nodes();
                    pbf_node->set_id(node->id);

                    pbf_node->set_lat(0); // TODO: encode node->lat
                    pbf_node->set_lon(0); // TODO: encode node->lon

                    for (int i=0, l = node->tag_count(); i < l; i++) {
                        pbf_node->add_keys(str2pbf(node->get_tag_key(i)));
                        pbf_node->add_vals(str2pbf(node->get_tag_value(i)));
                    }

                    OSMPBF::Info *pbf_node_info = pbf_node->mutable_info();
                    pbf_node_info->set_version((google::protobuf::int32) node->version);
                    pbf_node_info->set_timestamp((google::protobuf::int64) 0); // TODO: encode node->timestamp
                    pbf_node_info->set_changeset((google::protobuf::int64) node->changeset);
                    pbf_node_info->set_uid((google::protobuf::int64) node->uid);
                    pbf_node_info->set_user_sid(str2pbf(node->user));

                    check_block_contents_counter();
                }

                void write(Osmium::OSM::Way *way) {
                    if(!headerWritten) store_header_block();
                }

                void write(Osmium::OSM::Relation *relation) {
                    if(!headerWritten) store_header_block();
                }

                void write_final() {
                    store_primitive_block();
                    if(fd)
                        fclose(fd);
                }

            }; // class PBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBF_HPP
