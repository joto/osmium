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
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

/*

About the .osm.pbf file format
This is an excerpt of <http://wiki.openstreetmap.org/wiki/PBF_Format>

The .osm.pbf format and it's derived formats (.osh.pbf and .osc.pbf) are encoded
using googles protobuf library for the low-level storage. They are constructed
by nesting data on two levels:

On the lower level the file is constructed using BlobHeaders and Blobs. A .osm.pbf
file contains multiple sequences of
 1. a 4-byte header size, stored in network-byte-order
 2. a BlobHeader of exactly this size
 3. a Blob

The BlobHeader tells the reader about the type and size of the following Blob. The
Blob can contain data in raw or zlib-compressed form. After uncompressing the blob
it is treated differently depending on the type specified in the BlobHeader.

The contents of the Blob belongs to the higher level. It contains either an HeaderBlock
(type="OSMHeader") or an PrimitiveBlock (type="OSMData"). The file needs to have
at least one HeaderBlock before the first PrimitiveBlock.

The HeaderBlock contains meta-information like the writing program or a bbox. It may
also contain multiple "required features" that describe what kinds of input a
reading program needs to handle in order to fully understand the files' contents.

The PrimitiveBlock can store multiple types of objects (i.e. 5 nodes, 2 ways and
1 relation). It contains one or more PrimitiveGroup which in turn contain multiple
nodes, ways or relations. A PrimitiveGroup should only contain one kind of object.

There's a special kind of "object type" called dense-nodes. It is used to store nodes
in a very dense format, avoiding message overheads and using delta-encoding for nearly
all ids.

All Strings are stored as indexes to rows in a StringTable. The StringTable contains
one row for each used string, so strings that are used multiple times need to be
stored only once. The StringTable is sorted by usage-count, so the most often used
string is stored at index 1.

A simple outline of a .osm.pbf file could look like this:

  4-bytes header size
  BlobHeader
  Blob
    HeaderBlock
  4-bytes header size
  BlobHeader
  Blob
    PrimitiveBlock
      StringTable
      PrimitiveGroup
        5 nodes
      PrimitiveGroup
        2 ways
      PrimitiveGroup
        1 relation

More complete outlines of real .osm.pbf files can be created using the osmpbf-outline Tool:
 <https://github.com/MaZderMind/OSM-binary/tree/osmpbf-outline>
*/

// netinet provides the network-byte-order conversion function
#include <netinet/in.h>

// the algorithm-lib contains the sort functions
#include <algorithm>

namespace Osmium {

    namespace Output {

        namespace OSM {

            class PBF : public Base {

                /**
                 * nanodegree multiplier
                 *
                 * used in latlon2int while converting floating-point
                 * lat/lon to integers
                 */
                static const long int NANO = 1000 * 1000 * 1000;

                /**
                 * maximum number of items in a primitive block.
                 *
                 * the uncompressed length of a Blob *should* be less
                 * than 16 megabytes and *must* be less than 32 megabytes.
                 *
                 * a block may contain any number of entities, as long as
                 * the size limits for the surrounding blob are obeyed.
                 * However, for simplicity, the current osmosis (0.38)
                 * as well as the osmium implement implementation always
                 * uses at most 8k entities in a block.
                 */
                static const unsigned int max_block_contents = 8000;

                /**
                 * maximum number of bytes an uncompressed block may take
                 */
                static const int MAX_BLOB_SIZE = 32 * 1024 * 1024;

                /**
                 * the file descriptor of the output file
                 *
                 * all PBF-Blobs get serialized into this descriptor.
                 */
                FILE *fd;

                /**
                 * a boolean indicating id fd has been fopen'ed and should be closed again
                 */
                bool fd_opened;

                /**
                 * should nodes be serialized into the dense format?
                 *
                 * nodes can be encoded one of two ways, as a Node
                 * (use_dense_format_ = false) and a special dense format.
                 * In the dense format, all information is stored 'column wise',
                 * as an array of ID's, array of latitudes, and array of
                 * longitudes. Each column is delta-encoded. This reduces
                 * header overheads and allows delta-coding to work very effectively.
                 */
                bool use_dense_format_;

                /**
                 * should the PBF blobs contain zlib compressed data?
                 *
                 * the zlib compression is optional, it's possible to store the
                 * blobs in raw format. Disabling the compression can improve the
                 * writing speed a little but the output will be 2x to 3x bigger.
                 */
                bool use_compression_;

                /**
                 * to flexibly handle multiple resolutions, the granularity, or
                 * resolution used for representing locations is adjustable in
                 * multiples of 1 nanodegree. The default scaling factor is 100
                 * nanodegrees, corresponding to about ~1cm at the equator.
                 * These is the current resolution of the OSM database.
                 */
                int granularity_;

                /**
                 * the granularity used for representing timestamps is also adjustable in
                 * multiples of 1 millisecond. The default scaling factor is 1000
                 * milliseconds, which is the current resolution of the OSM database.
                 */
                int date_granularity_;

                /**
                 * while the .osm.pbf-format is able to carry all meta information, it is
                 * also able to omit this information to reduce size.
                 */
                bool omit_metadata_;

                /**
                 * protobuf-struct of a Blob
                 */
                OSMPBF::Blob pbf_blob;

                /**
                 * protobuf-struct of a BlobHeader
                 */
                OSMPBF::BlobHeader pbf_blob_header;

                /**
                 * protobuf-struct of a HeaderBlock
                 */
                OSMPBF::HeaderBlock pbf_header_block;

                /**
                 * protobuf-struct of a PrimitiveBlock
                 */
                OSMPBF::PrimitiveBlock pbf_primitive_block;

                /**
                 * pointer to PrimitiveGroups inside the current PrimitiveBlock,
                 * used for writing nodes, ways or relations
                 */
                OSMPBF::PrimitiveGroup *pbf_nodes;
                OSMPBF::PrimitiveGroup *pbf_ways;
                OSMPBF::PrimitiveGroup *pbf_relations;

                /**
                 * counter used to quickly check the number of objects stored inside
                 * the current PrimitiveBlock. When the counter reaches max_block_contents
                 * the PrimitiveBlock is serialized into a Blob and flushed to the file.
                 *
                 * this check is performed in check_block_contents_counter() which is
                 * called once for each object.
                 */
                unsigned int primitive_block_contents;

                /**
                 * this is the struct used to build the StringTable. It is stored as
                 * the value-part in the strings-map.
                 *
                 * when a new string is added to the map, its count is set to 0 and
                 * the interim_id is set to the current size of the map. This interim_id
                 * is then stored into the pbf-objects.
                 *
                 * before the PrimitiveBlock is serialized, the map is sorted by count
                 * and stored into the pbf-StringTable. Afterwards the interim-ids are
                 * mapped to the "real" id in the StringTable.
                 *
                 * this way often used strings get lower ids in the StringTable. As the
                 * protobuf-serializer stores numbers in variable bit-lengths, lower
                 * IDs means less used space in the resulting file.
                 */
                struct string_info {
                    /**
                     * number of occurrences of this string
                     */
                    unsigned int count;

                    /**
                     * an intermediate-id
                     */
                    unsigned int interim_id;
                };

                /**
                 * interim StringTable, storing all strings that should be written to
                 * the StringTable once the block is written to disk.
                 */
                std::map<std::string, string_info> strings;

                /**
                 * this map is used to map the interim-ids to real StringTable-IDs after
                 * writing all strings to the StringTable.
                 */
                std::map<unsigned int, unsigned int> string_ids_map;

                /**
                 * buffer used while compressing blobs
                 */
                char pack_buffer[MAX_BLOB_SIZE];

                /**
                 * this struct and its variable last_dense_info is used to calculate the
                 * delta-encoding while storing dense-nodes. It holds the last seen values
                 * from which the difference is stored into the protobuf.
                 */
                struct last_dense {
                    long int id;
                    long int lat;
                    long int lon;
                    long int timestamp;
                    long int changeset;
                    long int uid;
                    long int user_sid;
                } last_dense_info;


                ///// Blob writing /////

                /**
                 * serialize a protobuf-message together into a Blob, optionally apply compression
                 * and write it together with a BlobHeader to the file.
                 *
                 * type specifies the type-string used in the BlobHeader and msg the protobuf-message.
                 */
                void store_blob(const std::string &type, const google::protobuf::MessageLite &msg) {
                    // buffer to serialize the protobuf message to
                    std::string data;

                    // serialize the protobuf message to the string
                    msg.SerializeToString(&data);

                    // test if compression is enabled
                    if (use_compression()) {
                        // zlib compression context
                        z_stream z;

                        // next byte to compress
                        z.next_in   = (unsigned char*)  data.c_str();

                        // number of bytes to compress
                        z.avail_in  = data.size();

                        // place to store next compressed byte
                        z.next_out  = (unsigned char*)  pack_buffer;

                        // space for compressed data
                        z.avail_out = MAX_BLOB_SIZE;

                        // custom allocator functions - not used
                        z.zalloc    = Z_NULL;
                        z.zfree     = Z_NULL;
                        z.opaque    = Z_NULL;

                        // initiate the compression
                        if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
                            throw std::runtime_error("failed to init zlib stream");
                        }

                        // compress
                        if (deflate(&z, Z_FINISH) != Z_STREAM_END) {
                            throw std::runtime_error("failed to deflate zlib stream");
                        }

                        // finish compression
                        if (deflateEnd(&z) != Z_OK) {
                            throw std::runtime_error("failed to deinit zlib stream");
                        }

                        // print debug info about the compression
                        if (Osmium::global.debug) {
                            std::cerr << "pack " << data.size() << " bytes to " << z.total_out << " bytes (1:" << (double)data.size() / z.total_out << ")" << std::endl;
                        }

                        // set the compressed data on the Blob
                        pbf_blob.set_zlib_data(pack_buffer, z.total_out);
                    } else { // no compression
                        // print debug info about the raw data
                        if (Osmium::global.debug) {
                            std::cerr << "store uncompressed " << data.size() << " bytes" << std::endl;
                        }

                        // just set the raw data on the Blob
                        pbf_blob.set_raw(data);
                    }

                    // set the size of the uncompressed data on the blob
                    pbf_blob.set_raw_size(data.size());

                    // clear the blob string
                    data.clear();

                    // serialize and clear the Blob
                    pbf_blob.SerializeToString(&data);
                    pbf_blob.Clear();

                    // set the header-type to the supplied string on the BlobHeader
                    pbf_blob_header.set_type(type);

                    // set the size of the serialized blob on the BlobHeader
                    pbf_blob_header.set_datasize(data.size());

                    // a place to serialize the BlobHeader to
                    std::string blobhead;

                    // serialize and clear the BlobHeader
                    pbf_blob_header.SerializeToString(&blobhead);
                    pbf_blob_header.Clear();

                    // the 4-byte size of the BlobHeader, transformed from Host- to Network-Byte-Order
                    int32_t sz = htonl(blobhead.size());

                    // write to the file: the 4-byte BlobHeader-Size followed by the BlobHeader followed by the Blob
                    fwrite(&sz, sizeof(sz), 1, fd);
                    fwrite(blobhead.c_str(), blobhead.size(), 1, fd);
                    fwrite(data.c_str(), data.size(), 1, fd);
                }


                ///// StringTable management /////

                /**
                 * record a string in the interim StringTable if it's missing, otherwise just increase its counter,
                 * return the interim-id assigned to the string.
                 */
                unsigned int record_string(const std::string& string) {
                    // try to find the string in the interim StringTable
                    if (strings.count(string) > 0) {
                        // found, get a pointer to the associated string_info struct
                        string_info *info_p = &strings[string];

                        // increase the counter by one
                        info_p->count++;

                        // return the associated interim-id
                        //if (Osmium::global.debug) fprintf(stderr, "found string %s at interim-id %u\n", string.c_str(), info_p->interim_id);
                        return info_p->interim_id;
                    } else {
                        // not found, initialize a new string_info struct with the count set to 0 and the
                        // interim-id set to the current size +1
                        string_info info = {0, (unsigned int)strings.size()+1};

                        // store this string_info struct in the interim StringTable
                        strings[string] = info;

                        // debug-print and return the associated interim-id
                        if (Osmium::global.debug) {
                            std::cerr << "record string " << string << " at interim-id " << info.interim_id << std::endl;
                        }
                        return info.interim_id;
                    }
                }

                /**
                 * this is the comparator used while sorting the interim StringTable.
                 */
                static bool stringtable_comparator(const std::pair<std::string, string_info> &a, const std::pair<std::string, string_info> &b) {
                    // it first compares based on count
                    if (a.second.count > b.second.count) {
                        return true;
                    } else if (a.second.count < b.second.count) {
                        return false;
                    } else {
                        // if the count is equal, compare based on lexicography order so make
                        // the sorting of the later zlib compression faster
                        return a.first < b.first;
                    }
                }

                /**
                 * sort the interim StringTable and store it to the real protobuf StringTable.
                 * while storing to the real table, this function fills the string_ids_map with
                 * pairs, mapping the interim-ids to final and real StringTable ids.
                 */
                void store_stringtable() {
                    // as a map can't be sorted, we need a vector holding our string/string_info pairs
                    std::vector<std::pair<std::string, string_info>> strvec;

                    // we now copy the contents from the map over to the vector
                    std::copy(strings.begin(), strings.end(), back_inserter(strvec));

                    // next the vector is sorted using our comparator
                    std::sort(strvec.begin(), strvec.end(), stringtable_comparator);

                    // add a stringtable to the PrimitiveBlock and save a pointer
                    OSMPBF::StringTable *st = pbf_primitive_block.mutable_stringtable();

                    // iterate over the items of our vector
                    for (int i=0, l=strvec.size(); i<l; i++) {
                        if (Osmium::global.debug) {
                            std::cerr << "store stringtable: " << strvec[i].first << " (cnt=" << strvec[i].second.count+1 << ") with interim-id " << strvec[i].second.interim_id << " at stringtable-id " << i+1 << std::endl;
                        }

                        // add the string of the current item to the pbf StringTable
                        st->add_s(strvec[i].first);

                        // store the mapping from the interim-id to the real id
                        string_ids_map[strvec[i].second.interim_id] = i+1;
                    }
                }

                /**
                 * map from an interim-id to a real string-id, throwing an exception if an
                 * unknown mapping is requested.
                 */
                unsigned int map_string_id(const unsigned int interim_id) {
                    // declare an iterator over the ids-map
                    std::map<unsigned int, unsigned int>::const_iterator it;

                    // use the find method of the map to search for the mapping pair
                    it = string_ids_map.find(interim_id);

                    // if there was a hit
                    if (it != string_ids_map.end()) {
                        // return the real-id stored in the second part of the pair
                        //if (Osmium::global.debug) fprintf(stderr, "mapping interim-id %u to stringtable-id %u\n", interim_id, it->second);
                        return it->second;
                    }

                    // throw an exception
                    throw std::runtime_error("Request for string not in stringable\n");
                }

                /**
                 * before a PrimitiveBlock gets serialized, all interim StringTable-ids needs to be
                 * mapped to the associated real StringTable ids. Th is is done in this function.
                 *
                 * this function needs to know about the concrete structure of all item types to find
                 * all occurrences of string-ids.
                 */
                void map_string_ids() {
                    // test, if the node-block has been allocated
                    if (pbf_nodes) {
                        // iterate over all nodes, passing them to the map_common_string_ids function
                        for (int i=0, l=pbf_nodes->nodes_size(); i<l; i++) {
                            map_common_string_ids(pbf_nodes->mutable_nodes(i));
                        }

                        // test, if the node-block has a densenodes structure
                        if (pbf_nodes->has_dense()) {
                            // get a pointer to the densenodes structure
                            OSMPBF::DenseNodes *dense = pbf_nodes->mutable_dense();

                            // in the densenodes structure keys and vals are encoded in an intermixed
                            // array, individual nodes are seperated by a value of 0 (0 in the StringTable
                            // is always unused). String-ids of 0 are thus kept alone.
                            for (int i=0, l=dense->keys_vals_size(); i<l; i++) {
                                // map interim string-ids > 0 to real string ids
                                int sid = dense->keys_vals(i);
                                if (sid > 0) {
                                    dense->set_keys_vals(i, (google::protobuf::uint32) map_string_id((unsigned int) sid));
                                }
                            }

                            // test if the densenodes block has meta infos
                            if (dense->has_denseinfo()) {
                                // get a pointer to the denseinfo structure
                                OSMPBF::DenseInfo *denseinfo = dense->mutable_denseinfo();

                                // iterate over all username string-ids
                                for (int i=0, l= denseinfo->user_sid_size(); i<l; i++) {
                                    // map interim string-ids > 0 to real string ids
                                    unsigned int user_sid = map_string_id((unsigned int) denseinfo->user_sid(i));

                                    // delta encode the string-id
                                    denseinfo->set_user_sid(i, (google::protobuf::uint32) user_sid - last_dense_info.user_sid);

                                    // store the last string-id for the next delta-coding
                                    last_dense_info.user_sid = user_sid;
                                }
                            }
                        }
                    }

                    // test, if the ways-block has been allocated
                    if (pbf_ways) {
                        // iterate over all ways, passing them to the map_common_string_ids function
                        for (int i=0, l=pbf_ways->ways_size(); i<l; i++) {
                            map_common_string_ids(pbf_ways->mutable_ways(i));
                        }
                    }

                    // test, if the relations-block has been allocated
                    if (pbf_relations) {
                        // iterate over all relations
                        for (int i=0, l=pbf_relations->relations_size(); i<l; i++) {
                            // get a pointer to the relation
                            OSMPBF::Relation *relation = pbf_relations->mutable_relations(i);

                            // pass them to the map_common_string_ids function
                            map_common_string_ids(relation);

                            // iterate over all relation members, mapping the interim string-ids
                            // of the role to real string ids
                            for (int mi=0, ml=relation->roles_sid_size(); mi<ml; mi++)
                                relation->set_roles_sid(mi, (google::protobuf::uint32) map_string_id((unsigned int) relation->roles_sid(mi)));
                        }
                    }
                }

                /**
                 * a helper function used in map_string_ids to map common interim string-ids of the
                 * user name and all tags to real string ids.
                 *
                 * pbf_object_t is either OSMPBF::Node, OSMPBF::Way or OSMPBF::Relation.
                 */
                template <class pbf_object_t> void map_common_string_ids(pbf_object_t *in) {
                    // if the object has meta-info attached
                    if (in->has_info()) {
                        // map the interim-id of the user name to a real id
                        OSMPBF::Info *info = in->mutable_info();
                        info->set_user_sid((google::protobuf::uint32) map_string_id((unsigned int) info->user_sid()));
                    }

                    // iterate over all tags and map the interim-ids of the key and the value to real ids
                    for (int i=0, l=in->keys_size(); i<l; i++) {
                        in->set_keys(i, (google::protobuf::uint32) map_string_id((unsigned int) in->keys(i)));
                        in->set_vals(i, (google::protobuf::uint32) map_string_id((unsigned int) in->vals(i)));
                    }
                }




                ///// MetaData helper /////

                /**
                 * convert a double lat or lon value to an int, respecting the current blocks granularity
                 */
                long int latlon2int(double latlon) {
                    return (latlon * NANO / granularity());
                }

                /**
                 * convert a timestamp to an int, respecting the current blocks granularity
                 */
                long int timestamp2int(time_t timestamp) {
                    return timestamp * ((double)1000 / date_granularity());
                }

                /**
                 * helper function used in the write()-calls to apply common information from an osmium-object
                 * onto a pbf-object.
                 *
                 * pbf_object_t is either OSMPBF::Node, OSMPBF::Way or OSMPBF::Relation.
                 */
                template <class pbf_object_t> void apply_common_info(Osmium::OSM::Object *in, pbf_object_t *out) {
                    // set the object-id
                    out->set_id(in->get_id());

                    // iterate over all tags and set the keys and vals, recording the strings in the
                    // interim StringTable and storing the interim ids
                    for (int i=0, l=in->tag_count(); i<l; i++) {
                        out->add_keys(record_string(in->get_tag_key(i)));
                        out->add_vals(record_string(in->get_tag_value(i)));
                    }

                    if (!omit_metadata()) {
                        // add an info-section to the pbf object and set the meta-info on it
                        OSMPBF::Info *out_info = out->mutable_info();
                        out_info->set_version((google::protobuf::int32) in->get_version());
                        out_info->set_timestamp((google::protobuf::int64) timestamp2int(in->get_timestamp()));
                        out_info->set_changeset((google::protobuf::int64) in->get_changeset());
                        out_info->set_uid((google::protobuf::int64) in->get_uid());
                        out_info->set_user_sid(record_string(in->get_user()));
                    }
                }


                ///// High-Level Block writing /////

                /**
                 * store the current pbf_header_block into a Blob and clear this struct afterwards.
                 */
                void store_header_block() {
                    if (Osmium::global.debug) {
                        std::cerr << "storing header block" << std::endl;
                    }
                    store_blob("OSMHeader", pbf_header_block);
                    pbf_header_block.Clear();
                }

                /**
                 * store the interim StringTable to the current pbf_primitive_block, map all interim string ids
                 * to real StringTable ids and then store the current pbf_primitive_block into a Blob and clear
                 * this struct and all related pointers and maps afterwards.
                 */
                void store_primitive_block() {
                    if (Osmium::global.debug) {
                        std::cerr << "storing primitive block with " << primitive_block_contents << "items" << std::endl;
                    }

                    // store the interim StringTable into the protobuf object
                    store_stringtable();

                    // map all interim string ids to real ids
                    map_string_ids();

                    // store the Blob
                    store_blob("OSMData", pbf_primitive_block);

                    // clear the PrimitiveBlock struct
                    pbf_primitive_block.Clear();

                    // add empty StringTable entry at index 0
                    // StringTable index 0 is rserved as delimiter in the densenodes key/value list
                    // this line also ensures that there's always a valid StringTable
                    pbf_primitive_block.mutable_stringtable()->add_s("");

                    // set the granularity
                    pbf_primitive_block.set_granularity(granularity());
                    pbf_primitive_block.set_date_granularity(date_granularity());

                    // clear the interim StringTable and its id map
                    strings.clear();
                    string_ids_map.clear();

                    // reset the dense-info struct to zero
                    last_dense_info = {0, 0, 0, 0, 0, 0, 0};

                    // reset the contents-counter to zero
                    primitive_block_contents = 0;

                    // reset the node/way/relation pointers to NULL
                    pbf_nodes = NULL;
                    pbf_ways = NULL;
                    pbf_relations = NULL;
                }

                /**
                 * this little function checks primitive_block_contents counter against its maximum and calls
                 * store_primitive_block to flush the block to the disk when it's reached. It's also responsible
                 * for increasing this counter.
                 */
                void check_block_contents_counter() {
                    if (primitive_block_contents >= max_block_contents) {
                        store_primitive_block();
                    }
                    primitive_block_contents++;
                }

            public:

                /**
                 * default constructor, initializing the file descriptor to stdout
                 */
                PBF() : Base(),
                    fd(stdout),
                    fd_opened(false),
                    use_dense_format_(true),
                    use_compression_(true),
                    omit_metadata_(false),
                    pbf_nodes(NULL),
                    pbf_ways(NULL),
                    pbf_relations(NULL),
                    primitive_block_contents(0),
                    last_dense_info({0, 0, 0, 0, 0, 0, 0}) {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;

                    // set defaults from proto-definiton
                    granularity_ = pbf_primitive_block.granularity();
                    date_granularity_ = pbf_primitive_block.date_granularity();
                }

                /**
                 * filename constructor, opening the specified file for writing
                 */
                PBF(std::string &filename) : Base(),
                    fd_opened(false),
                    use_dense_format_(true),
                    use_compression_(true),
                    omit_metadata_(false),
                    pbf_nodes(NULL),
                    pbf_ways(NULL),
                    pbf_relations(NULL),
                    primitive_block_contents(0),
                    last_dense_info({0, 0, 0, 0, 0, 0, 0}) {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;

                    // set defaults from proto-definition
                    granularity_ = pbf_primitive_block.granularity();
                    date_granularity_ = pbf_primitive_block.date_granularity();

                    // open the file for writing
                    fd = fopen(filename.c_str(), "w");
                    if (!fd) {
                        throw std::runtime_error("unable to open outfile");
                    }
                }


                /**
                 * getter to check whether the densenodes-feature is used
                 */
                bool use_dense_format() const {
                    return use_dense_format_;
                }

                /**
                 * setter to set whether the densenodes-feature is used
                 */
                PBF& use_dense_format(bool d) {
                    use_dense_format_ = d;
                    return *this;
                }


                /**
                 * getter to check whether zlib-compression is used
                 */
                bool use_compression() const {
                    return use_compression_;
                }

                /**
                 * setter to set whether zlib-compression is used
                 */
                PBF& use_compression(bool d) {
                    use_compression_ = d;
                    return *this;
                }


                /**
                 * getter to access the granularity
                 */
                int granularity() const {
                    return granularity_;
                }

                /**
                 * setter to set the granularity
                 */
                PBF& granularity(int g) {
                    granularity_ = g;
                    return *this;
                }


                /**
                 * getter to access the date_granularity
                 */
                int date_granularity() const {
                    return date_granularity_;
                }

                /**
                 * setter to set whether zlib-compression is used
                 */
                PBF& date_granularity(int g) {
                    date_granularity_ = g;
                    return *this;
                }


                /**
                 * getter to check whether metadata is omitted
                 */
                bool omit_metadata() const {
                    return omit_metadata_;
                }

                /**
                 * setter to set whether to omit metadata
                 */
                PBF& omit_metadata(bool o) {
                    omit_metadata_ = o;
                    return *this;
                }


                /**
                 * initialize the writing process
                 *
                 * this initializes the header-block, sets the required-features and
                 * the writing-program and adds the obligatory StringTable-Index 0
                 */
                void write_init() {
                    if (Osmium::global.debug) {
                        std::cerr << "pbf write init" << std::endl;
                    }

                    // add the schema version as required feature to the HeaderBlock
                    pbf_header_block.add_required_features("OsmSchema-V0.6");

                    // when the densenodes-feature is used, add DenseNodes as required feature
                    if (use_dense_format())
                        pbf_header_block.add_required_features("DenseNodes");

                    // when the resulting file will carry history information, add
                    // HistoricalInformation as required feature
                    if (is_history_file())
                        pbf_header_block.add_required_features("HistoricalInformation");

                    // set the writing program
                    pbf_header_block.set_writingprogram("Osmium (http://wiki.openstreetmap.org/wiki/Osmium)");
                    store_header_block();

                    // add empty StringTable entry at index 0
                    // StringTable index 0 is rserved as delimiter in the densenodes key/value list
                    // this line also ensures that there's always a valid StringTable
                    pbf_primitive_block.mutable_stringtable()->add_s("");

                    // set the granularity
                    pbf_primitive_block.set_granularity(granularity());
                    pbf_primitive_block.set_date_granularity(date_granularity());
                }

                /**
                 * write bbox-information to the HeaderBlock
                 */
                void write_bounds(double minlon, double minlat, double maxlon, double maxlat) {
                    // add a HeaderBBox section to the HeaderBlock
                    OSMPBF::HeaderBBox *bbox = pbf_header_block.mutable_bbox();

                    // encode the bbox in nanodegrees
                    bbox->set_left(minlon * NANO);
                    bbox->set_top(minlat * NANO);
                    bbox->set_right(maxlon * NANO);
                    bbox->set_bottom(maxlat * NANO);
                }

                /**
                 * add a node to the pbf.
                 *
                 * a call to this method won't write the node to the file directly but
                 * cache it for later bulk-writing. Calling write_final ensures that everything
                 * gets written and every file pointer is closed.
                 */
                void write(Osmium::OSM::Node *node) {
                    // first of we check the contents-counter which may flush the cached nodes to
                    // disk if the limit is reached. This call also increases the contents-counter
                    check_block_contents_counter();

                    // if no PrimitiveGroup for nodes has been added, add one and save the pointer
                    if (!pbf_nodes)
                        pbf_nodes = pbf_primitive_block.add_primitivegroup();

                    // if the dense-format is disabled, use the classic format
                    if (!use_dense_format()) {
                        if (Osmium::global.debug) {
                            std::cerr << "node " << node->get_id() << " v" << node->get_version() << std::endl;
                        }

                        // add a "classic" node to the group
                        OSMPBF::Node *pbf_node = pbf_nodes->add_nodes();

                        // copy the common meta-info from the osmium-object to the pbf-object
                        apply_common_info(node, pbf_node);

                        // modify lat & lon to integers, respecting the block's granularity and copy
                        // the ints to the pbf-object
                        pbf_node->set_lat(latlon2int(node->get_lat()));
                        pbf_node->set_lon(latlon2int(node->get_lon()));
                    }

                    // use the dense-format
                    else {
                        if (Osmium::global.debug) {
                            std::cerr << "densenode " << node->get_id() << " v" << node->get_version() << std::endl;
                        }

                        // add a DenseNodes-Section to the PrimitiveGroup
                        OSMPBF::DenseNodes *dense = pbf_nodes->mutable_dense();

                        // copy the id, delta encoded against last_dense_info
                        long int id = node->get_id();
                        dense->add_id(id - last_dense_info.id);
                        last_dense_info.id = id;

                        // copy the latitude, delta encoded against last_dense_info
                        long int lat = latlon2int(node->get_lat());
                        dense->add_lat(lat - last_dense_info.lat);
                        last_dense_info.lat = lat;

                        // copy the longitude, delta encoded against last_dense_info
                        long int lon = latlon2int(node->get_lon());
                        dense->add_lon(lon - last_dense_info.lon);
                        last_dense_info.lon = lon;

                        // in the densenodes structure keys and vals are encoded in an intermixed
                        // array, individual nodes are seperated by a value of 0 (0 in the StringTable
                        // is always unused)
                        // so for three nodes the keys_vals array may look like this: 3 5 2 1 0 0 8 5
                        // the first node has two tags (3=>5 and 2=>1), the second node has does not
                        // have any tags and the third node has a single tag (8=>5)
                        for (int i=0, l=node->tag_count(); i<l; i++) {
                            dense->add_keys_vals(record_string(node->get_tag_key(i)));
                            dense->add_keys_vals(record_string(node->get_tag_value(i)));
                        }
                        dense->add_keys_vals(0);

                        if (!omit_metadata()) {
                            // add a DenseInfo-Section to the PrimitiveGroup
                            OSMPBF::DenseInfo *denseinfo = dense->mutable_denseinfo();

                            // copy the version
                            denseinfo->add_version(node->get_version());

                            // copy the timestamp, delta encoded against last_dense_info
                            long int timestamp = timestamp2int(node->get_timestamp());
                            denseinfo->add_timestamp(timestamp - last_dense_info.timestamp);
                            last_dense_info.timestamp = timestamp;

                            // copy the changeset, delta encoded against last_dense_info
                            long int changeset = node->get_changeset();
                            denseinfo->add_changeset(node->get_changeset() - last_dense_info.changeset);
                            last_dense_info.changeset = changeset;

                            // copy the user id, delta encoded against last_dense_info
                            long int uid = node->get_uid();
                            denseinfo->add_uid(uid - last_dense_info.uid);
                            last_dense_info.uid = uid;

                            // record the user-name to the interim stringtable and copy the
                            // interim string-id to the pbf-object
                            denseinfo->add_user_sid(record_string(node->get_user()));
                        }
                    }
                }

                /**
                 * add a way to the pbf.
                 *
                 * a call to this method won't write the way to the file directly but
                 * cache it for later bulk-writing. Calling write_final ensures that everything
                 * gets written and every file pointer is closed.
                 */
                void write(Osmium::OSM::Way *way) {
                    // first of we check the contents-counter which may flush the cached nodes to
                    // disk if the limit is reached. This call also increases the contents-counter
                    check_block_contents_counter();

                    if (Osmium::global.debug) {
                        std::cerr << "way " << way->get_id() << " v" << way->get_version() << " with " << way->node_count() << " nodes" << std::endl;
                    }

                    // if no PrimitiveGroup for nodes has been added, add one and save the pointer
                    if (!pbf_ways)
                        pbf_ways = pbf_primitive_block.add_primitivegroup();

                    // add a way to the group
                    OSMPBF::Way *pbf_way = pbf_ways->add_ways();

                    // copy the common meta-info from the osmium-object to the pbf-object
                    apply_common_info(way, pbf_way);

                    // last way-node-id used for delta-encoding
                    long int last_id = 0;

                    // iterate over all way-nodes
                    for (int i=0, l = way->node_count(); i<l; i++) {
                        // copy the way-node-id, delta encoded against last_id
                        long int id = way->get_node_id(i);
                        pbf_way->add_refs(id - last_id);
                        last_id = id;
                    }
                }

                /**
                 * add a relation to the pbf.
                 *
                 * a call to this method won't write the way to the file directly but
                 * cache it for later bulk-writing. Calling write_final ensures that everything
                 * gets written and every file pointer is closed.
                 */
                void write(Osmium::OSM::Relation *relation) {
                    // first of we check the contents-counter which may flush the cached nodes to
                    // disk if the limit is reached. This call also increases the contents-counter
                    check_block_contents_counter();

                    if (Osmium::global.debug) {
                        std::cerr << "relation " << relation->get_id() << " v" << relation->get_version() << " with " << relation->member_count() << " members" << std::endl;
                    }

                    // if no PrimitiveGroup for relations has been added, add one and save the pointer
                    if (!pbf_relations)
                        pbf_relations = pbf_primitive_block.add_primitivegroup();

                    // add a relation to the group
                    OSMPBF::Relation *pbf_relation = pbf_relations->add_relations();

                    // copy the common meta-info from the osmium-object to the pbf-object
                    apply_common_info(relation, pbf_relation);

                    // last relation-member-id used for delta-encoding
                    long int last_id = 0;

                    // iterate over all relation-members
                    for (int i=0, l=relation->member_count(); i<l; i++) {
                        // save a pointer to the osmium-object representing the relation-member
                        const Osmium::OSM::RelationMember *mem = relation->get_member(i);

                        // record the relation-member role to the interim stringtable and copy the
                        // interim string-id to the pbf-object
                        pbf_relation->add_roles_sid(record_string(mem->get_role()));

                        // copy the relation-member-id, delta encoded against last_id
                        long int id = mem->get_ref();
                        pbf_relation->add_memids(id - last_id);
                        last_id = id;

                        // copy the relation-member-type, mapped to the OSMPBF enum
                        switch(mem->get_type()) {
                            case 'n': pbf_relation->add_types(OSMPBF::Relation::NODE); break;
                            case 'w': pbf_relation->add_types(OSMPBF::Relation::WAY); break;
                            case 'r': pbf_relation->add_types(OSMPBF::Relation::RELATION); break;
                            default: throw std::runtime_error("Unknown relation member type: " + mem->get_type());
                        }
                    }
                }

                /**
                 * finalize the writing process, flush any open primitive blocks to the file and
                 * close the file descriptor that the constructor opened.
                 */
                void write_final() {
                    if (Osmium::global.debug) {
                        std::cerr << "finishing" << std::endl;
                    }

                    // if the current block contains any elementy, flush it to the protobuf
                    if (primitive_block_contents > 0) {
                        store_primitive_block();
                    }

                    // only close the fd if it has been opened by the constructor
                    if (fd_opened && fd) {
                        fclose(fd);
                    }
                }

            }; // class PBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBF_HPP
