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
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <string>
#include <zlib.h>
#include <typeinfo>

#include <osmpbf/osmpbf.h>

namespace Osmium {

    namespace Input {

        /**
        * Class for parsing PBF files.
        *
        * Generally you are not supposed to instantiate this class yourself.
        * Instead create an OSMFile object and call its read() method.
        *
        * @tparam THandler A handler class (subclass of Osmium::Handler::Base).
        */
        template <class THandler>
        class PBF : public Base<THandler> {

            char buffer[OSMPBF::max_uncompressed_blob_size];
            char unpack_buffer[OSMPBF::max_uncompressed_blob_size];

            typedef std::pair<const void*, size_t> array_t;

            OSMPBF::BlobHeader pbf_blob_header;
            OSMPBF::PrimitiveBlock pbf_primitive_block;
            int64_t date_factor;

        public:

            /**
            * Instantiate PBF Parser
            *
            * @param file OSMFile instance.
            * @param handler Instance of THandler. If NULL an instance of class THandler is created internally.
            */
            PBF(OSMFile& file, THandler& handler) : Base<THandler>(file, handler) {
                GOOGLE_PROTOBUF_VERIFY_VERSION;
            }

            /**
            * Parse PBF file.
            *
            * Will throw a subclass of Osmium::OSMFile::FileTypeError when it
            * turns out while parsing the file, that it is of the wrong type.
            */
            void parse() {
                try {
                    while (read_blob_header()) {
                        array_t a = read_blob(pbf_blob_header.datasize());

                        if (pbf_blob_header.type() == "OSMData") {
                            if (!pbf_primitive_block.ParseFromArray(a.first, a.second)) {
                                throw std::runtime_error("Failed to parse PrimitiveBlock.");
                            }
                            OSMPBF::StringTable stringtable = pbf_primitive_block.stringtable();
                            date_factor = pbf_primitive_block.date_granularity() / 1000;
                            for (int i=0; i < pbf_primitive_block.primitivegroup_size(); i++) {
                                parse_group(pbf_primitive_block.primitivegroup(i), stringtable);
                            }
                        } else if (pbf_blob_header.type() == "OSMHeader") {
                            OSMPBF::HeaderBlock pbf_header_block;
                            if (!pbf_header_block.ParseFromArray(a.first, a.second)) {
                                throw std::runtime_error("Failed to parse HeaderBlock.");
                            }

                            bool has_historical_information_feature = false;
                            for (int i=0; i < pbf_header_block.required_features_size(); i++) {
                                const std::string& feature = pbf_header_block.required_features(i);

                                if (feature == "OsmSchema-V0.6") continue;
                                if (feature == "DenseNodes") continue;
                                if (feature == "HistoricalInformation") {
                                    has_historical_information_feature = true;
                                    continue;
                                }

                                std::ostringstream errmsg;
                                errmsg << "Required feature not supported: " << feature;
                                throw std::runtime_error(errmsg.str());
                            }

                            Osmium::OSMFile::FileType* expected_file_type = this->get_file().get_type();
                            if (expected_file_type == Osmium::OSMFile::FileType::OSM() && has_historical_information_feature) {
                                throw Osmium::OSMFile::FileTypeOSMExpected();
                            }
                            if (expected_file_type == Osmium::OSMFile::FileType::History() && !has_historical_information_feature) {
                                throw Osmium::OSMFile::FileTypeHistoryExpected();
                            }
                            if (pbf_header_block.has_bbox()) {
                                OSMPBF::HeaderBBox bbox = pbf_header_block.bbox();
                                this->meta().bounds().extend(Osmium::OSM::Position((double)bbox.left() / OSMPBF::lonlat_resolution, (double)bbox.bottom() / OSMPBF::lonlat_resolution));
                                this->meta().bounds().extend(Osmium::OSM::Position((double)bbox.right() / OSMPBF::lonlat_resolution, (double)bbox.top() / OSMPBF::lonlat_resolution));
                            }
                        } else {
                            if (Osmium::debug()) {
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
            * and after_* methods are called.
            *
            * @param group The PrimitiveGroup to parse.
            * @param stringtable The string table inside the PrimitiveBlock with tags and usernames.
            */
            void parse_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                if (group.has_dense())  {
                    this->call_after_and_before_handlers(NODE);

                    // MAGIC: This bit of magic checks whether the empty node function in the
                    // handler base class was overwritten. If it was we parse the nodes from the input
                    // file, if not we skip parsing them because they will not be used anyway.
                    if (typeid(&THandler::node) != typeid(&Osmium::Handler::Base::node)) {
                        parse_dense_node_group(group, stringtable);
                    }
                } else if (group.ways_size() != 0) {
                    this->call_after_and_before_handlers(WAY);

                    // MAGIC: see above
                    if (typeid(&THandler::way) != typeid(&Osmium::Handler::Base::way)) {
                        parse_way_group(group, stringtable);
                    }
                } else if (group.relations_size() != 0) {
                    this->call_after_and_before_handlers(RELATION);

                    // MAGIC: see above
                    if (typeid(&THandler::relation) != typeid(&Osmium::Handler::Base::relation)) {
                        parse_relation_group(group, stringtable);
                    }
                } else if (group.nodes_size() != 0) {
                    this->call_after_and_before_handlers(NODE);

                    // MAGIC: see above
                    if (typeid(&THandler::node) != typeid(&Osmium::Handler::Base::node)) {
                        parse_node_group(group, stringtable);
                    }
                } else {
                    throw std::runtime_error("Group of unknown type.");
                }
            }

            void parse_node_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.nodes_size();
                for (int entity=0; entity < max_entity; entity++) {
                    this->node()->reset();

                    const OSMPBF::Node& pbf_node = group.nodes(entity);

                    this->node()->id(pbf_node.id());
                    if (pbf_node.has_info()) {
                        this->node()->version(pbf_node.info().version())
                        .changeset(pbf_node.info().changeset())
                        .timestamp(pbf_node.info().timestamp() * date_factor)
                        .uid(pbf_node.info().uid())
                        .user(stringtable.s(pbf_node.info().user_sid()).data());
                        if (pbf_node.info().has_visible()) {
                            this->node()->visible(pbf_node.info().visible());
                        }
                    }

                    Osmium::OSM::TagList& tags = this->node()->tags();
                    for (int tag=0; tag < pbf_node.keys_size(); tag++) {
                        tags.add(stringtable.s( pbf_node.keys( tag ) ).data(),
                                 stringtable.s( pbf_node.vals( tag ) ).data());
                    }

                    this->node()->position(Osmium::OSM::Position(
                                             ( (double) pbf_node.lon() * pbf_primitive_block.granularity() + pbf_primitive_block.lon_offset() ) / OSMPBF::lonlat_resolution,
                                             ( (double) pbf_node.lat() * pbf_primitive_block.granularity() + pbf_primitive_block.lat_offset() ) / OSMPBF::lonlat_resolution));
                    this->handle_node();
                }
            }

            void parse_way_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.ways_size();
                for (int entity=0; entity < max_entity; entity++) {
                    this->way()->reset();

                    const OSMPBF::Way& pbf_way = group.ways(entity);

                    this->way()->id(pbf_way.id());
                    if (pbf_way.has_info()) {
                        this->way()->version(pbf_way.info().version())
                        .changeset(pbf_way.info().changeset())
                        .timestamp(pbf_way.info().timestamp() * date_factor)
                        .uid(pbf_way.info().uid())
                        .user(stringtable.s(pbf_way.info().user_sid()).data());
                        if (pbf_way.info().has_visible()) {
                            this->node()->visible(pbf_way.info().visible());
                        }
                    }

                    Osmium::OSM::TagList& tags = this->way()->tags();
                    for (int tag=0; tag < pbf_way.keys_size(); tag++) {
                        tags.add(stringtable.s( pbf_way.keys( tag ) ).data(),
                                 stringtable.s( pbf_way.vals( tag ) ).data());
                    }

                    uint64_t ref = 0;
                    for (int i=0; i < pbf_way.refs_size(); i++) {
                        ref += pbf_way.refs(i);
                        this->way()->add_node(ref);
                    }

                    this->handle_way();
                }
            }

            void parse_relation_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable) {
                int max_entity = group.relations_size();
                for (int entity=0; entity < max_entity; entity++) {
                    this->relation()->reset();

                    const OSMPBF::Relation& pbf_relation = group.relations(entity);

                    this->relation()->id(pbf_relation.id());
                    if (pbf_relation.has_info()) {
                        this->relation()->version(pbf_relation.info().version())
                        .changeset(pbf_relation.info().changeset())
                        .timestamp(pbf_relation.info().timestamp() * date_factor)
                        .uid(pbf_relation.info().uid())
                        .user(stringtable.s(pbf_relation.info().user_sid()).data());
                        if (pbf_relation.info().has_visible()) {
                            this->node()->visible(pbf_relation.info().visible());
                        }
                    }
 
                    Osmium::OSM::TagList& tags = this->relation()->tags();
                    for (int tag=0; tag < pbf_relation.keys_size(); tag++) {
                        tags.add(stringtable.s( pbf_relation.keys(tag) ).data(),
                                 stringtable.s( pbf_relation.vals(tag) ).data());
                    }

                    uint64_t ref = 0;
                    for (int i=0; i < pbf_relation.types_size(); i++) {
                        char type = 'x';
                        switch (pbf_relation.types(i)) {
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
                        ref += pbf_relation.memids(i);
                        this->relation()->add_member(type, ref, stringtable.s( pbf_relation.roles_sid( i ) ).data());
                    }

                    this->handle_relation();
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

                const OSMPBF::DenseNodes& dense = group.dense();
                int max_entity = dense.id_size();
                for (int entity=0; entity < max_entity; ++entity) {
                    this->node()->reset();

                    last_dense_id += dense.id(entity);
                    this->node()->id(last_dense_id);

                    if (dense.has_denseinfo()) {
                        last_dense_changeset += dense.denseinfo().changeset(entity);
                        last_dense_timestamp += dense.denseinfo().timestamp(entity);
                        last_dense_uid       += dense.denseinfo().uid(entity);
                        last_dense_user_sid  += dense.denseinfo().user_sid(entity);

                        this->node()->version(dense.denseinfo().version(entity));
                        this->node()->changeset(last_dense_changeset);
                        this->node()->timestamp(last_dense_timestamp * date_factor);
                        this->node()->uid(last_dense_uid);
                        this->node()->user(stringtable.s(last_dense_user_sid).data());

                        if (dense.denseinfo().visible_size() > 0) {
                            this->node()->visible(dense.denseinfo().visible(entity));
                        }
                    }

                    last_dense_latitude  += dense.lat(entity);
                    last_dense_longitude += dense.lon(entity);
                    this->node()->position(Osmium::OSM::Position(
                                             ( (double) last_dense_longitude * pbf_primitive_block.granularity() + pbf_primitive_block.lon_offset() ) / OSMPBF::lonlat_resolution,
                                             ( (double) last_dense_latitude  * pbf_primitive_block.granularity() + pbf_primitive_block.lat_offset() ) / OSMPBF::lonlat_resolution));

                    while (last_dense_tag < dense.keys_vals_size()) {
                        int tag_key_pos = dense.keys_vals(last_dense_tag);

                        if (tag_key_pos == 0) {
                            last_dense_tag++;
                            break;
                        }
 
                        Osmium::OSM::TagList& tags = this->node()->tags();
                        tags.add(stringtable.s(tag_key_pos).data(),
                                 stringtable.s(dense.keys_vals(last_dense_tag+1)).data());

                        last_dense_tag += 2;
                    }

                    this->handle_node();
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
                int size = sizeof(size_in_network_byte_order);
                int offset = 0;
                while(offset < size) {
                    int nread = read(this->get_fd(), size_in_network_byte_order + offset, size - offset);
                    if (nread == -1) {
                        throw std::runtime_error("read error");
                    } else if (nread == 0) {
                        return false; // EOF
                    }
                    offset += nread;
                }

                size = convert_from_network_byte_order(size_in_network_byte_order);
                if (size > OSMPBF::max_blob_header_size || size < 0) {
                    std::ostringstream errmsg;
                    errmsg << "BlobHeader size invalid:" << size;
                    throw std::runtime_error(errmsg.str());
                }

                offset = 0;
                while(offset < size) {
                    int nread = read(this->get_fd(), buffer + offset, size - offset);
                    if (nread < 1) {
                        throw std::runtime_error("failed to read BlobHeader");
                    }
                    offset += nread;
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
                if (size < 0 || size > OSMPBF::max_uncompressed_blob_size) {
                    std::ostringstream errmsg;
                    errmsg << "invalid blob size: " << size;
                    throw std::runtime_error(errmsg.str());
                }
                int offset = 0;
                while(offset < size) {
                    int nread = read(this->get_fd(), buffer + offset, size - offset);
                    if (nread < 1) { 
                        throw std::runtime_error("failed to read blob");
                    }
                    offset += nread;
                }
                if (!blob.ParseFromArray(buffer, size)) {
                    throw std::runtime_error("failed to parse blob");
                }

                if (blob.has_raw()) {
                    return array_t(blob.raw().data(), blob.raw().size());
                } else if (blob.has_zlib_data()) {
                    unpack_with_zlib(blob.zlib_data().data(), blob.zlib_data().size(), blob.raw_size());
                    return array_t(unpack_buffer, blob.raw_size());
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
