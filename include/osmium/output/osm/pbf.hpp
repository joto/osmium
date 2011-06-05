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

#include <osmpbf/osmpbf.h>

// StringTable management
#include <osmium/utils/stringtable.hpp>

namespace Osmium {

    namespace Output {

        namespace OSM {

            class PBF : public Base {

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
                static const uint32_t max_block_contents = 8000;

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
                 * to flexibly handle multiple resolutions, the granularity, or
                 * resolution used for representing locations is adjustable in
                 * multiples of 1 nanodegree. The default scaling factor is 100
                 * nanodegrees, corresponding to about ~1cm at the equator.
                 * These is the current resolution of the OSM database.
                 */
                int m_location_granularity;

                /**
                 * the granularity used for representing timestamps is also adjustable in
                 * multiples of 1 millisecond. The default scaling factor is 1000
                 * milliseconds, which is the current resolution of the OSM database.
                 */
                int m_date_granularity;

                /**
                 * should nodes be serialized into the dense format?
                 *
                 * nodes can be encoded one of two ways, as a Node
                 * (m_use_dense_format = false) and a special dense format.
                 * In the dense format, all information is stored 'column wise',
                 * as an array of ID's, array of latitudes, and array of
                 * longitudes. Each column is delta-encoded. This reduces
                 * header overheads and allows delta-coding to work very effectively.
                 */
                bool m_use_dense_format;

                /**
                 * should the PBF blobs contain zlib compressed data?
                 *
                 * the zlib compression is optional, it's possible to store the
                 * blobs in raw format. Disabling the compression can improve the
                 * writing speed a little but the output will be 2x to 3x bigger.
                 */
                bool m_use_compression;

                /**
                 * While the .osm.pbf-format is able to carry all meta information, it is
                 * also able to omit this information to reduce size.
                 */
                bool m_should_add_metadata;

                /**
                 * counter used to quickly check the number of objects stored inside
                 * the current PrimitiveBlock. When the counter reaches max_block_contents
                 * the PrimitiveBlock is serialized into a Blob and flushed to the file.
                 *
                 * this check is performed in check_block_contents_counter() which is
                 * called once for each object.
                 */
                uint16_t primitive_block_contents;

                // StringTable management
                Osmium::StringTable string_table;

                /**
                 * buffer used while compressing blobs
                 */
                char pack_buffer[OSMPBF::max_uncompressed_blob_size];

                /**
                 * this struct and its variable last_dense_info is used to calculate the
                 * delta-encoding while storing dense-nodes. It holds the last seen values
                 * from which the difference is stored into the protobuf.
                 */
                struct last_dense {
                    int64_t id;

                    int32_t lat;
                    int32_t lon;

                    int64_t timestamp;
                    int64_t changeset;
                    int64_t uid;

                    uint32_t user_sid;
                } last_dense_info;


                ///// Blob writing /////

                /**
                 * take s string and pack it into the pack_buffer, returnung the number of
                 * compressed bytes
                 */
                size_t zlib_compress(std::string &in) {
                    // zlib compression context
                    z_stream z;

                    // next byte to compress
                    z.next_in   = (uint8_t*) in.c_str();

                    // number of bytes to compress
                    z.avail_in  = in.size();

                    // place to store next compressed byte
                    z.next_out  = (uint8_t*) pack_buffer;

                    // space for compressed data
                    z.avail_out = OSMPBF::max_uncompressed_blob_size;

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
                        std::cerr << "pack " << in.size() << " bytes to " << z.total_out << " bytes (1:" << (double)in.size() / z.total_out << ")" << std::endl;
                    }

                    // number of compressed bytes
                    return z.total_out;
                }

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

                    if (use_compression()) {
                        // compress using zlib
                        size_t out = zlib_compress(data);

                        // set the compressed data on the Blob
                        pbf_blob.set_zlib_data(pack_buffer, out);
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
                    if (::write(get_fd(), &sz, sizeof(sz)) < 0) {
                        throw std::runtime_error("file error");
                    }
                    if (::write(get_fd(), blobhead.c_str(), blobhead.size()) < 0) {
                        throw std::runtime_error("file error");
                    }
                    if (::write(get_fd(), data.c_str(), data.size()) < 0) {
                        throw std::runtime_error("file error");
                    }
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
                                uint16_t sid = dense->keys_vals(i);
                                if (sid > 0) {
                                    dense->set_keys_vals(i, string_table.map_string_id(sid));
                                }
                            }

                            // test if the densenodes block has meta infos
                            if (dense->has_denseinfo()) {
                                // get a pointer to the denseinfo structure
                                OSMPBF::DenseInfo *denseinfo = dense->mutable_denseinfo();

                                // iterate over all username string-ids
                                for (int i=0, l= denseinfo->user_sid_size(); i<l; i++) {
                                    // map interim string-ids > 0 to real string ids
                                    uint16_t user_sid = string_table.map_string_id(denseinfo->user_sid(i));

                                    // delta encode the string-id
                                    denseinfo->set_user_sid(i, user_sid - last_dense_info.user_sid);

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
                            for (int mi=0, ml=relation->roles_sid_size(); mi<ml; mi++) {
                                relation->set_roles_sid(mi, string_table.map_string_id(relation->roles_sid(mi)));
                            }
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
                        info->set_user_sid(string_table.map_string_id(info->user_sid()));
                    }

                    // iterate over all tags and map the interim-ids of the key and the value to real ids
                    for (int i=0, l=in->keys_size(); i<l; i++) {
                        in->set_keys(i, string_table.map_string_id(in->keys(i)));
                        in->set_vals(i, string_table.map_string_id(in->vals(i)));
                    }
                }




                ///// MetaData helper /////

                /**
                 * convert a double lat or lon value to an int, respecting the current blocks granularity
                 */
                int64_t latlon2int(double latlon) {
                    return (latlon * OSMPBF::lonlat_resolution / granularity());
                }

                /**
                 * convert a timestamp to an int, respecting the current blocks granularity
                 */
                int64_t timestamp2int(time_t timestamp) {
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
                        out->add_keys(string_table.record_string(in->get_tag_key(i)));
                        out->add_vals(string_table.record_string(in->get_tag_value(i)));
                    }

                    if (should_add_metadata()) {
                        // add an info-section to the pbf object and set the meta-info on it
                        OSMPBF::Info *out_info = out->mutable_info();
                        out_info->set_version(in->get_version());
                        out_info->set_timestamp(timestamp2int(in->get_timestamp()));
                        out_info->set_changeset(in->get_changeset());
                        out_info->set_uid(in->get_uid());
                        out_info->set_user_sid(string_table.record_string(in->get_user()));
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
                        std::cerr << "storing primitive block with " << primitive_block_contents << " items" << std::endl;
                    }

                    // store the interim StringTable into the protobuf object
                    string_table.store_stringtable(pbf_primitive_block.mutable_stringtable());

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
                    string_table.clear();

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


                ///// Block content writing /////

                /**
                 * add a node in classic mode to the block
                 */
                void write_classic(Osmium::OSM::Node *node) {
                    // add a "classic" node to the group
                    OSMPBF::Node *pbf_node = pbf_nodes->add_nodes();

                    // copy the common meta-info from the osmium-object to the pbf-object
                    apply_common_info(node, pbf_node);

                    // modify lat & lon to integers, respecting the block's granularity and copy
                    // the ints to the pbf-object
                    pbf_node->set_lat(latlon2int(node->get_lat()));
                    pbf_node->set_lon(latlon2int(node->get_lon()));
                }

                /**
                 * add a node in dense mode to the block
                 */
                void write_dense(Osmium::OSM::Node *node) {
                    // add a DenseNodes-Section to the PrimitiveGroup
                    OSMPBF::DenseNodes *dense = pbf_nodes->mutable_dense();

                    // copy the id, delta encoded against last_dense_info
                    int64_t id = node->get_id();
                    dense->add_id(id - last_dense_info.id);
                    last_dense_info.id = id;

                    // copy the latitude, delta encoded against last_dense_info
                    int32_t lat = latlon2int(node->get_lat());
                    dense->add_lat(lat - last_dense_info.lat);
                    last_dense_info.lat = lat;

                    // copy the longitude, delta encoded against last_dense_info
                    int32_t lon = latlon2int(node->get_lon());
                    dense->add_lon(lon - last_dense_info.lon);
                    last_dense_info.lon = lon;

                    // in the densenodes structure keys and vals are encoded in an intermixed
                    // array, individual nodes are seperated by a value of 0 (0 in the StringTable
                    // is always unused)
                    // so for three nodes the keys_vals array may look like this: 3 5 2 1 0 0 8 5
                    // the first node has two tags (3=>5 and 2=>1), the second node has does not
                    // have any tags and the third node has a single tag (8=>5)
                    for (int i=0, l=node->tag_count(); i<l; i++) {
                        dense->add_keys_vals(string_table.record_string(node->get_tag_key(i)));
                        dense->add_keys_vals(string_table.record_string(node->get_tag_value(i)));
                    }
                    dense->add_keys_vals(0);

                    if (should_add_metadata()) {
                        // add a DenseInfo-Section to the PrimitiveGroup
                        OSMPBF::DenseInfo *denseinfo = dense->mutable_denseinfo();

                        // copy the version
                        denseinfo->add_version(node->get_version());

                        // copy the timestamp, delta encoded against last_dense_info
                        int64_t timestamp = timestamp2int(node->get_timestamp());
                        denseinfo->add_timestamp(timestamp - last_dense_info.timestamp);
                        last_dense_info.timestamp = timestamp;

                        // copy the changeset, delta encoded against last_dense_info
                        int64_t changeset = node->get_changeset();
                        denseinfo->add_changeset(node->get_changeset() - last_dense_info.changeset);
                        last_dense_info.changeset = changeset;

                        // copy the user id, delta encoded against last_dense_info
                        int64_t uid = node->get_uid();
                        denseinfo->add_uid(uid - last_dense_info.uid);
                        last_dense_info.uid = uid;

                        // record the user-name to the interim stringtable and copy the
                        // interim string-id to the pbf-object
                        denseinfo->add_user_sid(string_table.record_string(node->get_user()));
                    }
                }

                /**
                 * add a way in classic mode to the block
                 */
                void write_classic(Osmium::OSM::Way *way) {
                    // add a way to the group
                    OSMPBF::Way *pbf_way = pbf_ways->add_ways();

                    // copy the common meta-info from the osmium-object to the pbf-object
                    apply_common_info(way, pbf_way);

                    // last way-node-id used for delta-encoding
                    int64_t last_id = 0;

                    // iterate over all way-nodes
                    for (int i=0, l = way->node_count(); i<l; i++) {
                        // copy the way-node-id, delta encoded against last_id
                        int64_t id = way->get_node_id(i);
                        pbf_way->add_refs(id - last_id);
                        last_id = id;
                    }
                }

                /**
                 * add a relation in classic mode to the block
                 */
                void write_classic(Osmium::OSM::Relation *relation) {
                    // add a relation to the group
                    OSMPBF::Relation *pbf_relation = pbf_relations->add_relations();

                    // copy the common meta-info from the osmium-object to the pbf-object
                    apply_common_info(relation, pbf_relation);

                    // last relation-member-id used for delta-encoding
                    int64_t last_id = 0;

                    // iterate over all relation-members
                    for (int i=0, l=relation->member_count(); i<l; i++) {
                        // save a pointer to the osmium-object representing the relation-member
                        const Osmium::OSM::RelationMember *mem = relation->get_member(i);

                        // record the relation-member role to the interim stringtable and copy the
                        // interim string-id to the pbf-object
                        pbf_relation->add_roles_sid(string_table.record_string(mem->get_role()));

                        // copy the relation-member-id, delta encoded against last_id
                        int64_t id = mem->get_ref();
                        pbf_relation->add_memids(id - last_id);
                        last_id = id;

                        // copy the relation-member-type, mapped to the OSMPBF enum
                        switch (mem->get_type()) {
                            case 'n':
                                pbf_relation->add_types(OSMPBF::Relation::NODE);
                                break;
                            case 'w':
                                pbf_relation->add_types(OSMPBF::Relation::WAY);
                                break;
                            case 'r':
                                pbf_relation->add_types(OSMPBF::Relation::RELATION);
                                break;
                            default:
                                throw std::runtime_error("Unknown relation member type: " + mem->get_type());
                        }
                    }
                }

            public:

                /**
                 * Create PBF output object from OSMFile.
                 */
                PBF(OSMFile& file) : Base(file),
                    pbf_nodes(NULL),
                    pbf_ways(NULL),
                    pbf_relations(NULL),
                    m_location_granularity(pbf_primitive_block.granularity()),
                    m_date_granularity(pbf_primitive_block.date_granularity()),
                    m_use_dense_format(true),
                    m_use_compression(true),
                    m_should_add_metadata(true),
                    primitive_block_contents(0),
                    string_table(),
                    last_dense_info( {0, 0, 0, 0, 0, 0, 0} ) {

                    GOOGLE_PROTOBUF_VERIFY_VERSION;
                }

                /**
                 * getter to check whether the densenodes-feature is used
                 */
                bool use_dense_format() const {
                    return m_use_dense_format;
                }

                /**
                 * setter to set whether the densenodes-feature is used
                 */
                PBF& use_dense_format(bool flag) {
                    m_use_dense_format = flag;
                    return *this;
                }


                /**
                 * getter to check whether zlib-compression is used
                 */
                bool use_compression() const {
                    return m_use_compression;
                }

                /**
                 * setter to set whether zlib-compression is used
                 */
                PBF& use_compression(bool flag) {
                    m_use_compression = flag;
                    return *this;
                }


                /**
                 * getter to access the granularity
                 */
                int granularity() const {
                    return m_location_granularity;
                }

                /**
                 * setter to set the granularity
                 */
                PBF& granularity(int g) {
                    m_location_granularity = g;
                    return *this;
                }


                /**
                 * getter to access the date_granularity
                 */
                int date_granularity() const {
                    return m_date_granularity;
                }

                /**
                 * Set date granularity.
                 */
                PBF& date_granularity(int g) {
                    m_date_granularity = g;
                    return *this;
                }


                /**
                 * Getter to check whether metadata should be added.
                 */
                bool should_add_metadata() const {
                    return m_should_add_metadata;
                }

                /**
                 * Setter to set whether to add metadata.
                 */
                PBF& should_add_metadata(bool flag) {
                    m_should_add_metadata = flag;
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
                    if (use_dense_format()) {
                        pbf_header_block.add_required_features("DenseNodes");
                    }

                    // when the resulting file will carry history information, add
                    // HistoricalInformation as required feature
                    if (m_file.get_type() == OSMFile::FileType::History()) {
                        pbf_header_block.add_required_features("HistoricalInformation");
                    }

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
                    bbox->set_left(  minlon * OSMPBF::lonlat_resolution);
                    bbox->set_top(   minlat * OSMPBF::lonlat_resolution);
                    bbox->set_right( maxlon * OSMPBF::lonlat_resolution);
                    bbox->set_bottom(maxlat * OSMPBF::lonlat_resolution);
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
                    if (!pbf_nodes) {
                        pbf_nodes = pbf_primitive_block.add_primitivegroup();
                    }

                    if (Osmium::global.debug) {
                        std::cerr << "node " << node->get_id() << " v" << node->get_version() << std::endl;
                    }

                    // if the dense-format is disabled, use the classic format
                    if (use_dense_format()) {
                        write_dense(node);
                    } else {
                        write_classic(node);
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
                    if (!pbf_ways) {
                        pbf_ways = pbf_primitive_block.add_primitivegroup();
                    }

                    write_classic(way);
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
                    if (!pbf_relations) {
                        pbf_relations = pbf_primitive_block.add_primitivegroup();
                    }

                    write_classic(relation);
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

                    m_file.close();
                }

            }; // class PBF

        } // namespace OSM

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_OSM_PBF_HPP
