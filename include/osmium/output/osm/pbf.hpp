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
#include <algorithm>

namespace Osmium {

    namespace Output {

        namespace OSM {

            class PBF : public Base {

                static const int NANO = 1000 * 1000 * 1000;
                static const unsigned int max_block_contents = 8000;
                FILE *fd;

                bool use_dense_format_;

                bool headerWritten;

                OSMPBF::Blob pbf_blob;
                OSMPBF::BlobHeader pbf_blob_header;

                OSMPBF::HeaderBlock pbf_header_block;
                OSMPBF::PrimitiveBlock pbf_primitive_block;

                std::vector<Osmium::OSM::Node*> nodes;
                std::vector<Osmium::OSM::Way*> ways;
                std::vector<Osmium::OSM::Relation*> relations;
                std::map<std::string, int> strings;

                static const int MAX_BLOB_SIZE = 32 * 1024 * 1024;
                char pack_buffer[MAX_BLOB_SIZE];

                void store_blob(const std::string &type, const std::string &data) {
                    z_stream z;

                    z.next_in   = (unsigned char*)  data.c_str();
                    z.avail_in  = data.size();
                    z.next_out  = (unsigned char*)  pack_buffer;
                    z.avail_out = MAX_BLOB_SIZE;
                    z.zalloc    = Z_NULL;
                    z.zfree     = Z_NULL;
                    z.opaque    = Z_NULL;

                    if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
                        throw std::runtime_error("failed to init zlib stream");
                    }
                    if (deflate(&z, Z_FINISH) != Z_STREAM_END) {
                        throw std::runtime_error("failed to deflate zlib stream");
                    }
                    if (deflateEnd(&z) != Z_OK) {
                        throw std::runtime_error("failed to deinit zlib stream");
                    }

                    fprintf(stderr, "pack %d bytes to %d bytes\n", data.size(), z.total_out);
                    pbf_blob.set_raw_size(data.size());
                    pbf_blob.set_zlib_data(pack_buffer, z.total_out);

                    std::string blob;

                    pbf_blob.SerializeToString(&blob);
                    pbf_blob.Clear();

                    pbf_blob_header.set_type(type);
                    pbf_blob_header.set_datasize(blob.size());

                    std::string blobhead;
                    pbf_blob_header.SerializeToString(&blobhead);
                    fprintf(stderr, "storing blob type=%s (header=%u bytes, raw=%u bytes, compressed=%lu)\n", pbf_blob_header.type().c_str(), blobhead.size(), blob.size(), z.total_out);
                    pbf_blob_header.Clear();

                    int32_t sz = htonl(blobhead.size());
                    fwrite(&sz, sizeof(sz), 1, fd);
                    fwrite(blobhead.c_str(), blobhead.size(), 1, fd);
                    fwrite(blob.c_str(), blob.size(), 1, fd);
                }

                void store_header_block() {
                    fprintf(stderr, "storing header block\n");
                    std::string header;
                    pbf_header_block.SerializeToString(&header);
                    pbf_header_block.Clear();

                    store_blob("OSMHeader", header);
                    headerWritten = true;
                }

                void record_string(const std::string& string) {
                    if(strings.count(string) > 0)
                        strings[string]++;
                    else
                        strings.insert( std::pair<std::string, int>(string, 0) );
                }

                unsigned int index_string(const std::string& str) {
                    const OSMPBF::StringTable st = pbf_primitive_block.stringtable();

                    for(int i = 0, l = st.s_size(); i<l; i++) {
                        if(st.s(i) == str) {
                            fprintf(stderr, "read stringtable: %s -> %d\n", str.c_str(), i);
                            return i;
                        }
                    }

                    throw std::runtime_error("Request for string not in stringable");
                    return 0;
                }

                static bool str_sorter(const std::pair<std::string, int> &a, const std::pair<std::string, int> b) {
                    if(a.second > b.second)
                        return true;
                    else if(a.second < b.second)
                        return false;
                    else
                        return a.first < b.first;
                }

                void sort_and_store_strings() {
                    std::vector<std::pair<std::string, int>> strvec;

                    std::copy(strings.begin(), strings.end(), back_inserter(strvec));
                    std::sort(strvec.begin(), strvec.end(), str_sorter);

                    OSMPBF::StringTable *st = pbf_primitive_block.mutable_stringtable();
                    for(int i = 0, l = strvec.size(); i<l; i++) {
                        fprintf(stderr, "store stringtable: %s (cnt=%d)\n", strvec[i].first.c_str(), strvec[i].second+1);
                        st->add_s(strvec[i].first);
                    }
                }

                int latlon2int(double latlon) {
                    return (latlon / pbf_primitive_block.granularity()) * (long int)1000000000;
                }

                int timestamp2int(time_t timestamp) {
                    return timestamp * (1000 / pbf_primitive_block.date_granularity());
                }

                void store_nodes_block() {
                    fprintf(stderr, "storing nodes block with %u nodes\n", nodes.size());
                    sort_and_store_strings();

                    OSMPBF::PrimitiveGroup *pbf_primitive_group = pbf_primitive_block.add_primitivegroup();
                    for(int i = 0, l = nodes.size(); i<l; i++) {
                        Osmium::OSM::Node *node = nodes[i];
                        OSMPBF::Node *pbf_node = pbf_primitive_group->add_nodes();

                        pbf_node->set_id(node->id);

                        pbf_node->set_lat(latlon2int(node->get_lat()));
                        pbf_node->set_lon(latlon2int(node->get_lon()));

                        for (int i=0, l = node->tag_count(); i < l; i++) {
                            pbf_node->add_keys(index_string(node->get_tag_key(i)));
                            pbf_node->add_vals(index_string(node->get_tag_value(i)));
                        }

                        OSMPBF::Info *pbf_node_info = pbf_node->mutable_info();
                        pbf_node_info->set_version((google::protobuf::int32) node->version);
                        pbf_node_info->set_timestamp((google::protobuf::int64) timestamp2int(node->timestamp));
                        pbf_node_info->set_changeset((google::protobuf::int64) node->changeset);
                        pbf_node_info->set_uid((google::protobuf::int64) node->uid);
                        pbf_node_info->set_user_sid(index_string(node->user));
                        delete node;
                    }
                    nodes.clear();
                    strings.clear();
                    store_primitive_block();
                }

                void store_ways_block() {
                    fprintf(stderr, "storing ways block with %u ways\n", ways.size());
                }

                void store_relations_block() {
                    fprintf(stderr, "storing relations block with %u relations\n", relations.size());
                }

                void store_primitive_block() {
                    fprintf(stderr, "storing primitive block\n");
                    std::string block;
                    pbf_primitive_block.SerializeToString(&block);
                    pbf_primitive_block.Clear();

                    // add empty string-table entry at index 0
                    pbf_primitive_block.mutable_stringtable()->add_s("");

                    store_blob("OSMData", block);
                }

                void check_block_contents_counter() {
                    if(nodes.size() >= max_block_contents)
                        store_nodes_block();

                    if(ways.size() >= max_block_contents)
                        store_ways_block();

                    if(relations.size() >= max_block_contents)
                        store_relations_block();
                }

            public:

                PBF() : Base(),
                    fd(stdout),
                    use_dense_format_(true),
                    headerWritten(false) {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;
                }

                PBF(std::string &filename) : Base(),
                    use_dense_format_(true),
                    headerWritten(false) {

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

                    for (int i=0, l = node->tag_count(); i < l; i++) {
                        record_string(node->get_tag_key(i));
                        record_string(node->get_tag_value(i));
                    }
                    record_string(node->user);

                    nodes.push_back(new Osmium::OSM::Node(*node));
                    check_block_contents_counter();
                }

                void write(Osmium::OSM::Way *way) {
                    if(!headerWritten) store_header_block();
                }

                void write(Osmium::OSM::Relation *relation) {
                    if(!headerWritten) store_header_block();
                }

                void write_final() {
                    if(nodes.size() > 0)
                        store_nodes_block();

                    if(ways.size() > 0)
                        store_ways_block();

                    if(relations.size() > 0)
                        store_relations_block();

                    if(fd)
                        fclose(fd);
                }

            }; // class PBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBF_HPP
