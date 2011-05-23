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

                /**
                 * Nanodegree multiplier
                 *
                 * used in latlon2int while converting floating-point lat/lon to integers
                 */
                static const long int NANO = 1000 * 1000 * 1000;

                /**
                 * Maximum number of items in a primitive block.
                 *
                 * The uncompressed length of a Blob *should* be less
                 * than 16 megabytes and *must* be less than 32 megabytes.
                 *
                 * A block may contain any number of entities, as long as
                 * the size limits for the surrounding blob are obeyed.
                 * However, for simplicity, the current osmosis (0.38)
                 * as well as the osmium implement implementation always
                 * uses at most 8k entities in a block.
                 */
                static const unsigned int max_block_contents = 8000;

                /**
                 * The file descriptor of the output file
                 *
                 * All PBF-Blobs get serialized into this descriptor.
                 */
                FILE *fd;

                /**
                 * Should nodes be serialized into the dense format?
                 *
                 * Nodes can be encoded one of two ways, as a Node
                 * (use_dense_format_ = false) and a special dense format.
                 * In the dense format, all information is stored 'columnwise',
                 * as an array of ID's, array of latitudes, and array of
                 * longitudes. Each column is delta-encoded. This reduces
                 * header overheads and allows delta-coding to work very effectively.
                 */
                bool use_dense_format_;

                /**
                 * Should the PBF blobs contain zlib compressed data?
                 *
                 * The zlib compression is optional, it's possible to store the 
                 * blobs in raw format. Disabling the compression can improve the 
                 * writing speed a little but the outout will be 2x to 3x bigger.
                 */
                bool use_compression_;

                OSMPBF::Blob pbf_blob;
                OSMPBF::BlobHeader pbf_blob_header;

                OSMPBF::HeaderBlock pbf_header_block;
                OSMPBF::PrimitiveBlock pbf_primitive_block;
                OSMPBF::PrimitiveGroup *pbf_nodes;
                OSMPBF::PrimitiveGroup *pbf_ways;
                OSMPBF::PrimitiveGroup *pbf_relations;
                unsigned int primitive_block_contents;

                struct string_info {
                    unsigned int count;
                    unsigned int interim_id;
                };

                std::map<std::string, string_info> strings;
                std::map<unsigned int, unsigned int> string_ids_map;

                static const int MAX_BLOB_SIZE = 32 * 1024 * 1024;
                char pack_buffer[MAX_BLOB_SIZE];

                struct last_dense {
                    long int id;
                    long int lat;
                    long int lon;
                    long int timestamp;
                    long int changeset;
                    long int uid;
                    long int user_sid;
                } last_dense_info;

                void store_blob(const std::string &type, const std::string &data) {
                    if(use_compression()) {
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

                        if(Osmium::global.debug) fprintf(stderr, "pack %lu bytes to %ld bytes (1:%f))\n", (long unsigned int)data.size(), z.total_out, (double)data.size() / z.total_out);
                        pbf_blob.set_zlib_data(pack_buffer, z.total_out);
                    }
                    else { // use_compression
                        pbf_blob.set_raw(data);
                    }

                    pbf_blob.set_raw_size(data.size());
                    std::string blob;

                    pbf_blob.SerializeToString(&blob);
                    pbf_blob.Clear();

                    pbf_blob_header.set_type(type);
                    pbf_blob_header.set_datasize(blob.size());

                    std::string blobhead;
                    pbf_blob_header.SerializeToString(&blobhead);
                    pbf_blob_header.Clear();

                    int32_t sz = htonl(blobhead.size());
                    fwrite(&sz, sizeof(sz), 1, fd);
                    fwrite(blobhead.c_str(), blobhead.size(), 1, fd);
                    fwrite(blob.c_str(), blob.size(), 1, fd);
                }

                void store_header_block() {
                    if(Osmium::global.debug) fprintf(stderr, "storing header block\n");
                    std::string header;
                    pbf_header_block.SerializeToString(&header);
                    pbf_header_block.Clear();

                    store_blob("OSMHeader", header);
                }

                unsigned int record_string(const std::string& string) {
                    if(strings.count(string) > 0) {
                        string_info *info_p;
                        info_p = &strings[string];
                        info_p->count++;

                        //if(Osmium::global.debug) fprintf(stderr, "found string %s at interim-id %u\n", string.c_str(), info_p->interim_id);
                        return info_p->interim_id;
                    } else {
                        string_info info;
                        info.count = 0;
                        info.interim_id = strings.size()+1;
                        strings[string] = info;

                        if(Osmium::global.debug) fprintf(stderr, "record string %s at interim-id %u\n", string.c_str(), info.interim_id);
                        return info.interim_id;
                    }
                }

                unsigned int map_string_index(const unsigned int interim_id) {
                    std::map<unsigned int, unsigned int>::const_iterator it;
                    it = string_ids_map.find(interim_id);
                    if(it != string_ids_map.end()) {
                        //if(Osmium::global.debug) fprintf(stderr, "mapping interim-id %u to stringtable-id %u\n", interim_id, it->second);
                        return it->second;
                    }

                    throw std::runtime_error("Request for string not in stringable\n");
                    return 0;
                }

                static bool str_sorter(const std::pair<std::string, string_info> &a, const std::pair<std::string, string_info> &b) {
                    if(a.second.count > b.second.count)
                        return true;
                    else if(a.second.count < b.second.count)
                        return false;
                    else
                        return a.first < b.first;
                }

                void sort_and_store_strings() {
                    std::vector<std::pair<std::string, string_info>> strvec;

                    std::copy(strings.begin(), strings.end(), back_inserter(strvec));
                    std::sort(strvec.begin(), strvec.end(), str_sorter);

                    string_ids_map.clear();
                    OSMPBF::StringTable *st = pbf_primitive_block.mutable_stringtable();
                    for(int i = 0, l = strvec.size(); i<l; i++) {
                        if(Osmium::global.debug) fprintf(stderr, "store stringtable: %s (cnt=%u) with interim-id %u at stringtable-id %u\n", strvec[i].first.c_str(), strvec[i].second.count+1, strvec[i].second.interim_id, i+1);
                        st->add_s(strvec[i].first);
                        string_ids_map[strvec[i].second.interim_id] = i+1;
                    }
                }

                void update_string_ids() {
                    if(pbf_nodes) {
                        for(int i = 0, l = pbf_nodes->nodes_size(); i<l; i++)
                            update_common_string_ids(pbf_nodes->mutable_nodes(i));

                        if(pbf_nodes->has_dense()) {
                            OSMPBF::DenseNodes *dense = pbf_nodes->mutable_dense();
                            OSMPBF::DenseInfo *denseinfo = dense->mutable_denseinfo();
                            for(int i = 0, l = dense->keys_vals_size(); i<l; i++) {
                                int sid = dense->keys_vals(i);
                                if(sid > 0)
                                    dense->set_keys_vals(i, (google::protobuf::uint32) map_string_index((unsigned int) sid));
                            }

                            for(int i = 0, l = denseinfo->user_sid_size(); i<l; i++) {
                                unsigned int user_sid = map_string_index((unsigned int) denseinfo->user_sid(i));
                                denseinfo->set_user_sid(i, (google::protobuf::uint32) user_sid - last_dense_info.user_sid);
                                last_dense_info.user_sid = user_sid;
                            }
                        }
                    }

                    if(pbf_ways) {
                        for(int i = 0, l = pbf_ways->ways_size(); i<l; i++)
                            update_common_string_ids(pbf_ways->mutable_ways(i));
                    }

                    if(pbf_relations) {
                        for(int i = 0, l = pbf_relations->relations_size(); i<l; i++) {
                            OSMPBF::Relation *relation = pbf_relations->mutable_relations(i);
                            update_common_string_ids(relation);

                            for (int mi = 0, ml = relation->roles_sid_size(); mi < ml; mi++)
                                relation->set_roles_sid(mi, (google::protobuf::uint32) map_string_index((unsigned int) relation->roles_sid(mi)));
                        }
                    }
                }

                template <class pbf_object_t> void update_common_string_ids(pbf_object_t *in) {
                    OSMPBF::Info *info = in->mutable_info();

                    info->set_user_sid((google::protobuf::uint32) map_string_index((unsigned int) info->user_sid()));
                    for (int i=0, l = in->keys_size(); i < l; i++) {
                        in->set_keys(i, (google::protobuf::uint32) map_string_index((unsigned int) in->keys(i)));
                        in->set_vals(i, (google::protobuf::uint32) map_string_index((unsigned int) in->vals(i)));
                    }
                }

                long int latlon2int(double latlon) {
                    return (latlon * NANO / pbf_primitive_block.granularity());
                }

                long int timestamp2int(time_t timestamp) {
                    return timestamp * (1000 / pbf_primitive_block.date_granularity());
                }

                template <class pbf_object_t> void apply_info(Osmium::OSM::Object *in, pbf_object_t *out) {
                    out->set_id(in->get_id());

                    for (int i=0, l = in->tag_count(); i < l; i++) {
                        out->add_keys(record_string(in->get_tag_key(i)));
                        out->add_vals(record_string(in->get_tag_value(i)));
                    }

                    OSMPBF::Info *out_info = out->mutable_info();
                    out_info->set_version((google::protobuf::int32) in->get_version());
                    out_info->set_timestamp((google::protobuf::int64) timestamp2int(in->get_timestamp()));
                    out_info->set_changeset((google::protobuf::int64) in->get_changeset());
                    out_info->set_uid((google::protobuf::int64) in->get_uid());
                    out_info->set_user_sid(record_string(in->get_user()));
                }

                void store_primitive_block() {
                    if(Osmium::global.debug) fprintf(stderr, "storing primitive block with %u items\n", primitive_block_contents);

                    sort_and_store_strings();
                    update_string_ids();

                    std::string block;
                    pbf_primitive_block.SerializeToString(&block);
                    store_blob("OSMData", block);

                    pbf_primitive_block.Clear();
                    strings.clear();
                    string_ids_map.clear();
                    last_dense_info = {0, 0, 0, 0, 0, 0, 0};
                    primitive_block_contents = 0;

                    // add empty string-table entry at index 0
                    pbf_primitive_block.mutable_stringtable()->add_s("");

                    // add a primitive-group to the block
                    pbf_nodes = NULL;
                    pbf_ways = NULL;
                    pbf_relations = NULL;
                }

                void check_block_contents_counter() {
                    if(primitive_block_contents >= max_block_contents)
                        store_primitive_block();

                    primitive_block_contents++;
                }

            public:

                PBF() : Base(),
                    fd(stdout),
                    use_dense_format_(true),
                    use_compression_(true),
                    pbf_nodes(NULL),
                    pbf_ways(NULL),
                    pbf_relations(NULL),
                    primitive_block_contents(0),
                    last_dense_info({0, 0, 0, 0, 0, 0, 0}) {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;
                }

                PBF(std::string &filename) : Base(),
                    use_dense_format_(true),
                    use_compression_(true),
                    pbf_nodes(NULL),
                    pbf_ways(NULL),
                    pbf_relations(NULL),
                    primitive_block_contents(0),
                    last_dense_info({0, 0, 0, 0, 0, 0, 0}) {

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

                bool use_compression() const {
                    return use_compression_;
                }

                PBF& use_compression(bool d) {
                    use_compression_ = d;
                    return *this;
                }

                void write_init() {
                    if(Osmium::global.debug) fprintf(stderr, "pbf write init\n");
                    pbf_header_block.add_required_features("OsmSchema-V0.6");

                    if(use_dense_format())
                        pbf_header_block.add_required_features("DenseNodes");

                    if(is_history_file())
                        pbf_header_block.add_required_features("HistoricalInformation");

                    pbf_header_block.set_writingprogram("Osmium (http://wiki.openstreetmap.org/wiki/Osmium)");
                    store_header_block();

                    // add empty string-table entry at index 0
                    pbf_primitive_block.mutable_stringtable()->add_s("");
                }

                void write_bounds(double minlon, double minlat, double maxlon, double maxlat) {
                    // FIXME: untested
                    OSMPBF::HeaderBBox *bbox = pbf_header_block.mutable_bbox();
                    bbox->set_left(minlon * NANO);
                    bbox->set_top(minlat * NANO);
                    bbox->set_right(maxlon * NANO);
                    bbox->set_bottom(maxlat * NANO);
                }

                void write(Osmium::OSM::Node *node) {
                    check_block_contents_counter();

                    if(!pbf_nodes)
                        pbf_nodes = pbf_primitive_block.add_primitivegroup();

                    if(!use_dense_format()) {
                        if(Osmium::global.debug) fprintf(stderr, "node %d v%d\n", node->get_id(), node->get_version());
                        OSMPBF::Node *pbf_node = pbf_nodes->add_nodes();
                        apply_info(node, pbf_node);

                        pbf_node->set_lat(latlon2int(node->get_lat()));
                        pbf_node->set_lon(latlon2int(node->get_lon()));
                    }

                    else { // use_dense_format
                        if(Osmium::global.debug) fprintf(stderr, "densenode %d v%d\n", node->get_id(), node->get_version());
                        OSMPBF::DenseNodes *dense = pbf_nodes->mutable_dense();
                        OSMPBF::DenseInfo *denseinfo = dense->mutable_denseinfo();

                        long int id = node->get_id();
                        dense->add_id(id - last_dense_info.id);
                        last_dense_info.id = id;

                        long int lat = latlon2int(node->get_lat());
                        dense->add_lat(lat - last_dense_info.lat);
                        last_dense_info.lat = lat;

                        long int lon = latlon2int(node->get_lon());
                        dense->add_lon(lon - last_dense_info.lon);
                        last_dense_info.lon = lon;

                        for (int i=0, l = node->tag_count(); i < l; i++) {
                            dense->add_keys_vals(record_string(node->get_tag_key(i)));
                            dense->add_keys_vals(record_string(node->get_tag_value(i)));
                        }
                        dense->add_keys_vals(0);

                        denseinfo->add_version(node->get_version());

                        long int timestamp = timestamp2int(node->get_timestamp());
                        denseinfo->add_timestamp(timestamp - last_dense_info.timestamp);
                        last_dense_info.timestamp = timestamp;

                        long int changeset = node->get_changeset();
                        denseinfo->add_changeset(node->get_changeset() - last_dense_info.changeset);
                        last_dense_info.changeset = changeset;

                        long int uid = node->get_uid();
                        denseinfo->add_uid(uid - last_dense_info.uid);
                        last_dense_info.uid = uid;

                        denseinfo->add_user_sid(record_string(node->get_user()));
                    }
                }

                void write(Osmium::OSM::Way *way) {
                    check_block_contents_counter();
                    if(Osmium::global.debug) fprintf(stderr, "way %d v%d with %lu nodes\n", way->get_id(), way->get_version(), (long unsigned int)way->node_count());

                    if(!pbf_ways)
                        pbf_ways = pbf_primitive_block.add_primitivegroup();

                    OSMPBF::Way *pbf_way = pbf_ways->add_ways();
                    apply_info(way, pbf_way);

                    long int last_id = 0;
                    for (int i=0, l = way->node_count(); i < l; i++) {
                        long int id = way->get_node_id(i);
                        pbf_way->add_refs(id - last_id);
                        last_id = id;
                    }
                }

                void write(Osmium::OSM::Relation *relation) {
                    check_block_contents_counter();
                    if(Osmium::global.debug) fprintf(stderr, "relation %d v%d with %lu members\n", relation->get_id(), relation->get_version(), (long unsigned int)relation->member_count());

                    if(!pbf_relations)
                        pbf_relations = pbf_primitive_block.add_primitivegroup();

                    OSMPBF::Relation *pbf_relation = pbf_relations->add_relations();
                    apply_info(relation, pbf_relation);

                    long int last_id = 0;
                    for (int i=0, l = relation->member_count(); i < l; i++) {
                        const Osmium::OSM::RelationMember *mem = relation->get_member(i);

                        pbf_relation->add_roles_sid(record_string(mem->get_role()));

                        long int id = mem->get_ref();
                        pbf_relation->add_memids(id - last_id);
                        last_id = id;

                        switch(mem->get_type()) {
                            case 'n': pbf_relation->add_types(OSMPBF::Relation::NODE); break;
                            case 'w': pbf_relation->add_types(OSMPBF::Relation::WAY); break;
                            case 'r': pbf_relation->add_types(OSMPBF::Relation::RELATION); break;
                            default: throw std::runtime_error("Unknown relation member type: " + mem->get_type());
                        }
                    }
                }

                void write_final() {
                    if(Osmium::global.debug) fprintf(stderr, "finishing\n");
                    if(primitive_block_contents > 0)
                        store_primitive_block();

                    if(fd)
                        fclose(fd);
                }

            }; // class PBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBF_HPP
