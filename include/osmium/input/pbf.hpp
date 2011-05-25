#ifndef OSMIUM_INPUT_PBF_HPP
#define OSMIUM_INPUT_PBF_HPP

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

#include <string>
#include <zlib.h>
#include <typeinfo>

#include <osmpbf/fileformat.pb.h>
#include <osmpbf/osmformat.pb.h>

namespace Osmium {

    namespace Input {

        /**
        * Class for parsing PBF files
        */
        template <class THandler>
        class PBF : public Base<THandler> {

            static const int NANO = 1000 * 1000 * 1000;
            static const int MAX_BLOB_HEADER_SIZE = 64 * 1024;
            static const int MAX_BLOB_SIZE = 32 * 1024 * 1024;

            char buffer[MAX_BLOB_SIZE];
            char unpack_buffer[MAX_BLOB_SIZE];

            int fd; ///< The file descriptor we are reading the data from.

            typedef struct {
                const void *data;
                size_t size;
            } array_t;

            OSMPBF::BlobHeader pbf_blob_header;
            OSMPBF::PrimitiveBlock pbf_primitive_block;

        public:

            /**
            * Instantiate PBF Parser
            *
            * @param in_fd File descripter to read data from.
            * @param h Instance of THandler or NULL.
            */
            PBF(int in_fd, THandler *h) __attribute__((noinline)) : Base<THandler>(h), fd(in_fd) {
                GOOGLE_PROTOBUF_VERIFY_VERSION;
            }

            /**
            * Parse PBF file.
            */
            void parse() __attribute__((noinline)) {
                try {
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
                            if (Osmium::global.debug) {
                                std::cerr << "Ignoring unknown blob type (" << pbf_blob_header.type().data() << ")." << std::endl;
                            }
                        }
                    }
                    this->call_after_and_before_handlers(UNKNOWN);
                } catch (Osmium::Input::StopReading) {
                    // if a handler says to stop reading, we do
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
                    this->call_after_and_before_handlers(NODE);

                    // MAGIC: This bit of magic checks whether the empty callback_node function in the
                    // handler base class was overwritten. If it was we parse the nodes from the input
                    // file, if not we skip parsing them because they will not be used anyway.
                    if (typeid(&THandler::callback_node) != typeid(&Osmium::Handler::Base::callback_node)) {
                        parse_dense_node_group(group, stringtable);
                    }
                } else if (group.ways_size() != 0) {
                    this->call_after_and_before_handlers(WAY);

                    // MAGIC: see above
                    if (typeid(&THandler::callback_way) != typeid(&Osmium::Handler::Base::callback_way)) {
                        parse_way_group(group, stringtable);
                    }
                } else if (group.relations_size() != 0) {
                    this->call_after_and_before_handlers(RELATION);

                    // MAGIC: see above
                    if (typeid(&THandler::callback_relation) != typeid(&Osmium::Handler::Base::callback_relation)) {
                        parse_relation_group(group, stringtable);
                    }
                } else if (group.nodes_size() != 0) {
                    this->call_after_and_before_handlers(NODE);

                    // MAGIC: see above
                    if (typeid(&THandler::callback_node) != typeid(&Osmium::Handler::Base::callback_node)) {
                        parse_node_group(group, stringtable);
                    }
                } else {
                    throw std::runtime_error("Group of unknown type.");
                }
            }

            void parse_node_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.nodes_size();
                for (int entity=0; entity < max_entity; entity++) {
                    this->node->reset();

                    const OSMPBF::Node& inputNode = group.nodes(entity);

                    this->node->set_id(inputNode.id()).
                                set_version(inputNode.info().version()).
                                set_changeset(inputNode.info().changeset()).
                                set_timestamp(inputNode.info().timestamp()).
                                set_uid(inputNode.info().uid()).
                                set_user(stringtable.s(inputNode.info().user_sid()).data());

                    for (int tag=0; tag < inputNode.keys_size(); tag++) {
                        this->node->add_tag(stringtable.s( inputNode.keys( tag ) ).data(),
                                            stringtable.s( inputNode.vals( tag ) ).data());
                    }

                    this->node->set_coordinates(( ( double ) inputNode.lon() * pbf_primitive_block.granularity() + pbf_primitive_block.lon_offset() ) / NANO,
                                                ( ( double ) inputNode.lat() * pbf_primitive_block.granularity() + pbf_primitive_block.lat_offset() ) / NANO);

                    this->handler->callback_node(this->node);
                }
            }

            void parse_way_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.ways_size();
                for (int entity=0; entity < max_entity; entity++) {
                    this->way->reset();

                    const OSMPBF::Way& inputWay = group.ways(entity);

                    this->way->set_id(inputWay.id()).
                               set_version(inputWay.info().version()).
                               set_changeset(inputWay.info().changeset()).
                               set_timestamp(inputWay.info().timestamp()).
                               set_uid(inputWay.info().uid()).
                               set_user(stringtable.s(inputWay.info().user_sid()).data());

                    for (int tag=0; tag < inputWay.keys_size(); tag++) {
                        this->way->add_tag(stringtable.s( inputWay.keys( tag ) ).data(),
                                           stringtable.s( inputWay.vals( tag ) ).data());
                    }

                    uint64_t lastRef = 0;
                    for (int i=0; i < inputWay.refs_size(); i++) {
                        lastRef += inputWay.refs( i );
                        this->way->add_node( lastRef );
                    }

                    this->handler->callback_way(this->way);
                }
            }

            void parse_relation_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.relations_size();
                for (int entity=0; entity < max_entity; entity++) {
                    this->relation->reset();

                    const OSMPBF::Relation& inputRelation = group.relations(entity);

                    this->relation->set_id(inputRelation.id()).
                                    set_version(inputRelation.info().version()).
                                    set_changeset(inputRelation.info().changeset()).
                                    set_timestamp(inputRelation.info().timestamp()).
                                    set_uid(inputRelation.info().uid()).
                                    set_user(stringtable.s(inputRelation.info().user_sid()).data());

                    for (int tag=0; tag < inputRelation.keys_size(); tag++) {
                        this->relation->add_tag(stringtable.s( inputRelation.keys(tag) ).data(),
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
                        this->relation->add_member(type, lastRef, stringtable.s( inputRelation.roles_sid( i ) ).data());
                    }

                    this->handler->callback_relation(this->relation);
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
                    this->node->reset();

                    const OSMPBF::DenseNodes& dense = group.dense();
                    last_dense_id        += dense.id(entity);
                    last_dense_latitude  += dense.lat(entity);
                    last_dense_longitude += dense.lon(entity);
                    last_dense_uid       += dense.denseinfo().uid(entity);
                    last_dense_user_sid  += dense.denseinfo().user_sid(entity);
                    last_dense_changeset += dense.denseinfo().changeset(entity);
                    last_dense_timestamp += dense.denseinfo().timestamp(entity);

                    this->node->set_id(last_dense_id);
                    this->node->set_version(dense.denseinfo().version(entity));
                    this->node->set_changeset(last_dense_changeset);
                    this->node->set_timestamp(last_dense_timestamp);
                    this->node->set_uid(last_dense_uid);
                    this->node->set_user(stringtable.s(last_dense_user_sid).data());

                    this->node->set_coordinates(( ( double ) last_dense_longitude * pbf_primitive_block.granularity() + pbf_primitive_block.lon_offset() ) / NANO,
                                                ( ( double ) last_dense_latitude  * pbf_primitive_block.granularity() + pbf_primitive_block.lat_offset() ) / NANO);

                    while (last_dense_tag < dense.keys_vals_size()) {
                        int tagValue = dense.keys_vals(last_dense_tag);

                        if (tagValue == 0) {
                            last_dense_tag++;
                            break;
                        }

                        this->node->add_tag(stringtable.s( dense.keys_vals(last_dense_tag  ) ).data(),
                                            stringtable.s( dense.keys_vals(last_dense_tag+1) ).data());

                        last_dense_tag += 2;
                    }

                    this->handler->callback_node(this->node);
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
            void unpack_with_zlib(const char *data, size_t size, size_t raw_size) const {
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

#endif // OSMIUM_INPUT_PBF_HPP
