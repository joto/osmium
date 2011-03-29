/*

The code in this file is based on the MoNav code, although it
has changed quite a bit since then.

For MoNav see http://wiki.openstreetmap.org/wiki/MoNav .

Copyright 2010  Christian Vetter veaac.fdirct@gmail.com

MoNav is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MoNav is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MoNav. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PBFPARSER_HPP
#define PBFPARSER_HPP

#include <string>
#include <zlib.h>

#include <fileformat.pb.h>
#include <osmformat.pb.h>

extern bool debug;

namespace Osmium {

    namespace Input {

        /**
        * Class for parsing PBF files
        */
        class PBF : public Base {

            static const int NANO = 1000 * 1000 * 1000;
            static const int MAX_BLOB_HEADER_SIZE = 64 * 1024;
            static const int MAX_BLOB_SIZE = 32 * 1024 * 1024;

            char buffer[MAX_BLOB_SIZE];
            char unpack_buffer[MAX_BLOB_SIZE];

            int fd; ///< The file descriptor we are reading the data from.
            struct callbacks *callbacks; ///< Functions to call for each object etc.

            typedef struct {
                const void *data;
                size_t size;
            } array_t;

            int groups_with_nodes; ///< Number of groups containing nodes.
            int groups_with_ways; ///< Number of groups containing ways.
            int groups_with_relations; ///< Number of groups containing relations.

            OSMPBF::BlobHeader pbf_blob_header;
            OSMPBF::PrimitiveBlock pbf_primitive_block;

        public:

            /**
            * Instantiate PBF Parser
            *
            * @param in_fd File descripter to read data from.
            * @param cb Structure with callbacks.
            */
            PBF(int in_fd, struct callbacks *cb) : Base(), fd(in_fd), callbacks(cb) {
                GOOGLE_PROTOBUF_VERIFY_VERSION;
                groups_with_nodes     = 0;
                groups_with_ways      = 0;
                groups_with_relations = 0;
            }

            ~PBF () {
                google::protobuf::ShutdownProtobufLibrary();
            }

            /**
            * Parse PBF file.
            */
            void parse() {
                while (read_blob_header()) {
                    array_t a = read_blob(pbf_blob_header.datasize());

                    if (pbf_blob_header.type() == "OSMData") {
                        if (!pbf_primitive_block.ParseFromArray(a.data, a.size)) {
                            throw std::runtime_error("Failed to parse PrimitiveBlock.");
                        }
                        OSMPBF::StringTable stringtable = pbf_primitive_block.stringtable();
                        for (int i=0; i < pbf_primitive_block.primitivegroup_size(); i++) {
                            parse_group(pbf_primitive_block.primitivegroup(i), stringtable);
                        }
                    } else if (pbf_blob_header.type() == "OSMHeader") {
                        OSMPBF::HeaderBlock pbf_header_block;
                        if (!pbf_header_block.ParseFromArray(a.data, a.size)) {
                            throw std::runtime_error("Failed to parse HeaderBlock.");
                        }

                        for (int i=0; i < pbf_header_block.required_features_size(); i++) {
                            const std::string& feature = pbf_header_block.required_features(i);

                            if ((feature != "OsmSchema-V0.6") && (feature != "DenseNodes")) {
                                std::ostringstream errmsg;
                                errmsg << "Required feature not supported: " << feature;
                                throw std::runtime_error(errmsg.str());
                            }
                        }
                    } else {
                        if (debug) {
                            std::cerr << "Ignoring unknown blob type (" << pbf_blob_header.type().data() << ")." << std::endl;
                        }
                    }
                }
                if (groups_with_nodes > 0 && groups_with_ways == 0 && groups_with_relations == 0) {
                    if (callbacks->after_nodes) { callbacks->after_nodes(); }
                }
                if (groups_with_ways > 0 && groups_with_relations == 0) {
                    if (callbacks->after_ways) { callbacks->after_ways(); }
                }
                if (groups_with_relations > 0) {
                    if (callbacks->after_relations) { callbacks->after_relations(); }
                }
            }

        private:

            /**
            * Parse one PrimitiveGroup inside a PrimitiveBlock. This function will check what
            * type of data the group contains (nodes, dense nodes, ways, or relations) and
            * call the proper parsing function. It will also make sure the right before_*
            * and after_* callbacks are called.
            *
            * @param group The PrimitiveGroup to parse.
            * @param stringtable The string table inside the PrimitiveBlock with tags and usernames.
            */
            void parse_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                if (group.has_dense())  {
                    if (groups_with_nodes == 0) {
                        if (callbacks->before_nodes) { callbacks->before_nodes(); }
                    }
                    groups_with_nodes++;
                    if (callbacks->node) { parse_dense_node_group(group, stringtable); }
                } else if (group.ways_size() != 0) {
                    if (groups_with_ways == 0) {
                        if (groups_with_nodes > 0) {
                            if (callbacks->after_nodes) { callbacks->after_nodes(); }
                        }
                        if (callbacks->before_ways) { callbacks->before_ways(); }
                    }
                    groups_with_ways++;
                    if (callbacks->way) { parse_way_group(group, stringtable); }
                } else if (group.relations_size() != 0) {
                    if (groups_with_relations == 0) {
                        if (groups_with_nodes > 0 && groups_with_ways == 0) {
                            if (callbacks->after_nodes) { callbacks->after_nodes(); }
                        }
                        if (groups_with_ways > 0) {
                            if (callbacks->after_ways) { callbacks->after_ways(); }
                        }
                        if (callbacks->before_relations) { callbacks->before_relations(); }
                    }
                    groups_with_relations++;
                    if (callbacks->relation) { parse_relation_group(group, stringtable); }
                } else if (group.nodes_size() != 0) {
                    if (groups_with_nodes == 0) {
                        if (callbacks->before_nodes) { callbacks->before_nodes(); }
                    }
                    groups_with_nodes++;
                    if (callbacks->node) { parse_node_group(group, stringtable); }
                } else {
                    throw std::runtime_error("Group of unknown type.");
                }
            }

            void parse_node_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.nodes_size();
                for (int entity=0; entity < max_entity; entity++) {
                    node->reset();

                    const OSMPBF::Node& inputNode = group.nodes(entity);
                    node->id = inputNode.id();
                    node->version = inputNode.info().version();
                    node->uid = inputNode.info().uid();
                    if (! memccpy(node->user, stringtable.s(inputNode.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                        throw std::length_error("user name too long");
                    }
                    node->changeset = inputNode.info().changeset();
                    node->timestamp = inputNode.info().timestamp();
                    for (int tag=0; tag < inputNode.keys_size(); tag++) {
                        node->add_tag(stringtable.s( inputNode.keys( tag ) ).data(),
                                    stringtable.s( inputNode.vals( tag ) ).data());
                    }

                    node->set_coordinates(( ( double ) inputNode.lon() * pbf_primitive_block.granularity() + pbf_primitive_block.lon_offset() ) / NANO,
                                        ( ( double ) inputNode.lat() * pbf_primitive_block.granularity() + pbf_primitive_block.lat_offset() ) / NANO);

                    if (callbacks->node) { callbacks->node(node); }
                }
            }

            void parse_way_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.ways_size();
                for (int entity=0; entity < max_entity; entity++) {
                    way->reset();

                    const OSMPBF::Way& inputWay = group.ways(entity);
                    way->id = inputWay.id();
                    way->version = inputWay.info().version();
                    way->uid = inputWay.info().uid();
                    if (! memccpy(way->user, stringtable.s(inputWay.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                        throw std::length_error("user name too long");
                    }
                    way->changeset = inputWay.info().changeset();
                    way->timestamp = inputWay.info().timestamp();
                    for (int tag=0; tag < inputWay.keys_size(); tag++) {
                        way->add_tag(stringtable.s( inputWay.keys( tag ) ).data(),
                                    stringtable.s( inputWay.vals( tag ) ).data());
                    }

                    uint64_t lastRef = 0;
                    for (int i=0; i < inputWay.refs_size(); i++) {
                        lastRef += inputWay.refs( i );
                        way->add_node( lastRef );
                    }

                    if (callbacks->way) { callbacks->way(way); }
                }
            }

            void parse_relation_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.relations_size();
                for (int entity=0; entity < max_entity; entity++) {
                    relation->reset();

                    const OSMPBF::Relation& inputRelation = group.relations(entity);
                    relation->id = inputRelation.id();
                    relation->version = inputRelation.info().version();
                    relation->uid = inputRelation.info().uid();
                    if (! memccpy(relation->user, stringtable.s(inputRelation.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                        throw std::length_error("user name too long");
                    }
                    relation->changeset = inputRelation.info().changeset();
                    relation->timestamp = inputRelation.info().timestamp();
                    for (int tag=0; tag < inputRelation.keys_size(); tag++) {
                        relation->add_tag(stringtable.s( inputRelation.keys(tag) ).data(),
                                        stringtable.s( inputRelation.vals(tag) ).data());
                    }

                    uint64_t lastRef = 0;
                    for (int i=0; i < inputRelation.types_size(); i++) {
                        char type = 'x';
                        switch (inputRelation.types(i)) {
                            case OSMPBF::Relation::NODE:
                                type = 'n';
                                break;
                            case OSMPBF::Relation::WAY:
                                type = 'w';
                                break;
                            case OSMPBF::Relation::RELATION:
                                type = 'r';
                                break;
                        }
                        lastRef += inputRelation.memids(i);
                        relation->add_member(type, lastRef, stringtable.s( inputRelation.roles_sid( i ) ).data());
                    }

                    if (callbacks->relation) { callbacks->relation(relation); }
                }
            }

            void parse_dense_node_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int64_t last_dense_id        = 0;
                int64_t last_dense_latitude  = 0;
                int64_t last_dense_longitude = 0;
                int64_t last_dense_uid       = 0;
                int64_t last_dense_user_sid  = 0;
                int64_t last_dense_changeset = 0;
                int64_t last_dense_timestamp = 0;
                int     last_dense_tag       = 0;

                int max_entity = group.dense().id_size();
                for (int entity=0; entity < max_entity; entity++) {
                    node->reset();

                    const OSMPBF::DenseNodes& dense = group.dense();
                    last_dense_id        += dense.id(entity);
                    last_dense_latitude  += dense.lat(entity);
                    last_dense_longitude += dense.lon(entity);
                    last_dense_uid       += dense.denseinfo().uid(entity);
                    last_dense_user_sid  += dense.denseinfo().user_sid(entity);
                    last_dense_changeset += dense.denseinfo().changeset(entity);
                    last_dense_timestamp += dense.denseinfo().timestamp(entity);
                    node->id      = last_dense_id;
                    node->uid     = last_dense_uid;
                    node->version = dense.denseinfo().version(entity);
                    if (! memccpy(node->user, stringtable.s(last_dense_user_sid).data(), 0, Osmium::OSM::Object::max_length_username)) {
                        throw std::length_error("user name too long");
                    }
                    node->changeset = last_dense_changeset;
                    node->timestamp = last_dense_timestamp;
                    node->set_coordinates(( ( double ) last_dense_longitude * pbf_primitive_block.granularity() + pbf_primitive_block.lon_offset() ) / NANO,
                                        ( ( double ) last_dense_latitude  * pbf_primitive_block.granularity() + pbf_primitive_block.lat_offset() ) / NANO);

                    while (last_dense_tag < dense.keys_vals_size()) {
                        int tagValue = dense.keys_vals(last_dense_tag);

                        if (tagValue == 0) {
                            last_dense_tag++;
                            break;
                        }

                        node->add_tag(stringtable.s( dense.keys_vals(last_dense_tag  ) ).data(),
                                    stringtable.s( dense.keys_vals(last_dense_tag+1) ).data());

                        last_dense_tag += 2;
                    }

                    if (callbacks->node) { callbacks->node(node); }
                }
            }

            /**
            * Convert 4 bytes from network byte order.
            */
            int convert_from_network_byte_order(unsigned char data[4]) {
                return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
            }

            /**
            * Read blob header by first reading the size and then the header
            *
            * @returns false for EOF, true otherwise
            */
            bool read_blob_header() {
                unsigned char size_in_network_byte_order[4];
                ssize_t bytes_read = read(fd, size_in_network_byte_order, sizeof(size_in_network_byte_order));
                if (bytes_read != sizeof(size_in_network_byte_order)) {
                    if (bytes_read == 0) {
                        return false; // EOF
                    }
                    throw std::runtime_error("read error");
                }

                int size = convert_from_network_byte_order(size_in_network_byte_order);
                if (size > MAX_BLOB_HEADER_SIZE || size < 0) {
                    std::ostringstream errmsg;
                    errmsg << "BlobHeader size invalid:" << size;
                    throw std::runtime_error(errmsg.str());
                }

                if (read(fd, buffer, size) != size) {
                    throw std::runtime_error("failed to read BlobHeader");
                }

                if (!pbf_blob_header.ParseFromArray(buffer, size)) {
                    throw std::runtime_error("failed to parse BlobHeader");
                }
                return true;
            }

            /**
            * Read a (possibly compressed) blob of data. If the blob is compressed, it is uncompressed.
            */
            array_t read_blob(int size) {
                static OSMPBF::Blob blob;
                if (size < 0 || size > MAX_BLOB_SIZE) {
                    std::ostringstream errmsg;
                    errmsg << "invalid blob size: " << size;
                    throw std::runtime_error(errmsg.str());
                }
                if (read(fd, buffer, size) != size) {
                    throw std::runtime_error("failed to read blob");
                }
                if (!blob.ParseFromArray(buffer, size)) {
                    throw std::runtime_error("failed to parse blob");
                }

                if (blob.has_raw()) {
                    return { blob.raw().data(), blob.raw().size() };
                } else if (blob.has_zlib_data()) {
                    unpack_with_zlib(blob.zlib_data().data(), blob.zlib_data().size(), blob.raw_size());
                    return { unpack_buffer, blob.raw_size() };
                } else if (blob.has_lzma_data()) {
                    throw std::runtime_error("lzma blobs not implemented");
                } else {
                    throw std::runtime_error("Blob contains no data");
                }
            }

            /**
            * Unpack a blob thats been packed with zlib.
            *
            * @param data Source (packed) data.
            * @param size Size of source data in bytes.
            * @param raw_size Size of destination (unpacked) data.
            */
            void unpack_with_zlib(const char *data, size_t size, size_t raw_size) {
                z_stream compressed_stream;

                compressed_stream.next_in   = (unsigned char*) data;
                compressed_stream.avail_in  = size;
                compressed_stream.next_out  = (unsigned char*) unpack_buffer;
                compressed_stream.avail_out = raw_size;
                compressed_stream.zalloc    = Z_NULL;
                compressed_stream.zfree     = Z_NULL;
                compressed_stream.opaque    = Z_NULL;

                if (inflateInit(&compressed_stream) != Z_OK) {
                    throw std::runtime_error("failed to init zlib stream");
                }
                if (inflate(&compressed_stream, Z_FINISH) != Z_STREAM_END) {
                    throw std::runtime_error("failed to inflate zlib stream");
                }
                if (inflateEnd(&compressed_stream) != Z_OK) {
                    throw std::runtime_error("failed to deinit zlib stream");
                }
            }

        }; // class PBF

    } // namespace Input

} // namespace Osmium

#endif // PBFPARSER_HPP
