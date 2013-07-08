#ifndef OSMIUM_INPUT_PBF_HPP
#define OSMIUM_INPUT_PBF_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#define OSMIUM_LINK_WITH_LIBS_PBF -lz -lpthread -lprotobuf-lite -losmpbf

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <zlib.h>

#include <osmpbf/osmpbf.h>

#include <osmium/input.hpp>

namespace Osmium {

    namespace Input {

        /**
        * Class for parsing PBF files.
        *
        * Generally you are not supposed to instantiate this class yourself.
        * Use the Osmium::Input::read() function instead.
        *
        * @tparam THandler A handler class (subclass of Osmium::Handler::Base).
        */
        template <class THandler>
        class PBF : public Base<THandler> {

            typedef std::pair<const void*, size_t> array_t;

            unsigned char m_input_buffer[OSMPBF::max_uncompressed_blob_size];
            unsigned char m_unpack_buffer[OSMPBF::max_uncompressed_blob_size];

            OSMPBF::Blob           m_pbf_blob;
            OSMPBF::BlobHeader     m_pbf_blob_header;
            OSMPBF::PrimitiveBlock m_pbf_primitive_block;

            int64_t m_date_factor;

        public:

            /**
            * Instantiate PBF Parser
            *
            * @param file OSMFile instance.
            * @param handler Instance of THandler.
            */
            PBF(const OSMFile& file, THandler& handler) :
                Base<THandler>(file, handler),
                m_input_buffer(),
                m_unpack_buffer(),
                m_pbf_blob(),
                m_pbf_blob_header(),
                m_pbf_primitive_block(),
                m_date_factor() {
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
                        const array_t a = read_blob(m_pbf_blob_header.datasize());

                        if (m_pbf_blob_header.type() == "OSMData") {
                            if (!m_pbf_primitive_block.ParseFromArray(a.first, a.second)) {
                                throw std::runtime_error("Failed to parse PrimitiveBlock.");
                            }
                            const OSMPBF::StringTable& stringtable = m_pbf_primitive_block.stringtable();
                            m_date_factor = m_pbf_primitive_block.date_granularity() / 1000;
                            for (int i=0; i < m_pbf_primitive_block.primitivegroup_size(); ++i) {
                                parse_group(m_pbf_primitive_block.primitivegroup(i), stringtable);
                            }
                        } else if (m_pbf_blob_header.type() == "OSMHeader") {
                            OSMPBF::HeaderBlock pbf_header_block;
                            if (!pbf_header_block.ParseFromArray(a.first, a.second)) {
                                throw std::runtime_error("Failed to parse HeaderBlock.");
                            }

                            bool has_historical_information_feature = false;
                            for (int i=0; i < pbf_header_block.required_features_size(); ++i) {
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

                            const Osmium::OSMFile::FileType* expected_file_type = this->file().type();
                            if (expected_file_type == Osmium::OSMFile::FileType::OSM() && has_historical_information_feature) {
                                throw Osmium::OSMFile::FileTypeOSMExpected();
                            }
                            if (expected_file_type == Osmium::OSMFile::FileType::History() && !has_historical_information_feature) {
                                throw Osmium::OSMFile::FileTypeHistoryExpected();
                            }

                            if (pbf_header_block.has_writingprogram()) {
                                this->meta().generator(pbf_header_block.writingprogram());
                            }
                            if (pbf_header_block.has_bbox()) {
                                const OSMPBF::HeaderBBox& bbox = pbf_header_block.bbox();
                                const int64_t resolution_convert = OSMPBF::lonlat_resolution / Osmium::OSM::coordinate_precision;
                                this->meta().bounds().extend(Osmium::OSM::Position(bbox.left()  / resolution_convert, bbox.bottom() / resolution_convert));
                                this->meta().bounds().extend(Osmium::OSM::Position(bbox.right() / resolution_convert, bbox.top()    / resolution_convert));
                            }
                        } else {
//                            std::cerr << "Ignoring unknown blob type (" << m_pbf_blob_header.type().data() << ").\n";
                        }
                    }
                    this->call_after_and_before_on_handler(UNKNOWN);
                } catch (Osmium::Handler::StopReading) {
                    // if a handler says to stop reading, we do
                }
                this->call_final_on_handler();
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
                    this->call_after_and_before_on_handler(NODE);
                    parse_dense_node_group(group, stringtable, &THandler::node);
                } else if (group.ways_size() != 0) {
                    this->call_after_and_before_on_handler(WAY);
                    parse_way_group(group, stringtable, &THandler::way);
                } else if (group.relations_size() != 0) {
                    this->call_after_and_before_on_handler(RELATION);
                    parse_relation_group(group, stringtable, &THandler::relation);
                } else if (group.nodes_size() != 0) {
                    this->call_after_and_before_on_handler(NODE);
                    parse_node_group(group, stringtable, &THandler::node);
                } else {
                    throw std::runtime_error("Group of unknown type.");
                }
            }

            // empty specialization to optimize the case where the node() method on the handler is empty
            void parse_node_group(const OSMPBF::PrimitiveGroup& /*group*/, const OSMPBF::StringTable& /*stringtable*/,
                                  void (Osmium::Handler::Base::*)(const shared_ptr<Osmium::OSM::Node const>&) const) {
            }

            template <typename T>
            void parse_node_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable, T) {
                int max_entity = group.nodes_size();
                for (int entity=0; entity < max_entity; ++entity) {
                    Osmium::OSM::Node& node = this->prepare_node();

                    const OSMPBF::Node& pbf_node = group.nodes(entity);

                    node.id(pbf_node.id());
                    if (pbf_node.has_info()) {
                        node.version(pbf_node.info().version())
                        .changeset(pbf_node.info().changeset())
                        .timestamp(pbf_node.info().timestamp() * m_date_factor)
                        .uid(pbf_node.info().uid())
                        .user(stringtable.s(pbf_node.info().user_sid()).data());
                        if (pbf_node.info().has_visible()) {
                            node.visible(pbf_node.info().visible());
                        }
                    }

                    Osmium::OSM::TagList& tags = node.tags();
                    for (int tag=0; tag < pbf_node.keys_size(); ++tag) {
                        tags.add(stringtable.s(pbf_node.keys(tag)).data(),
                                 stringtable.s(pbf_node.vals(tag)).data());
                    }

                    node.position(Osmium::OSM::Position(
                                      (pbf_node.lon() * m_pbf_primitive_block.granularity() + m_pbf_primitive_block.lon_offset()) / (OSMPBF::lonlat_resolution / Osmium::OSM::coordinate_precision),
                                      (pbf_node.lat() * m_pbf_primitive_block.granularity() + m_pbf_primitive_block.lat_offset()) / (OSMPBF::lonlat_resolution / Osmium::OSM::coordinate_precision)));
                    this->call_node_on_handler();
                }
            }

            // empty specialization to optimize the case where the way() method on the handler is empty
            void parse_way_group(const OSMPBF::PrimitiveGroup& /*group*/, const OSMPBF::StringTable& /*stringtable*/,
                                 void (Osmium::Handler::Base::*)(const shared_ptr<Osmium::OSM::Way const>&) const) {
            }

            template <typename T>
            void parse_way_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable, T) {
                int max_entity = group.ways_size();
                for (int entity=0; entity < max_entity; ++entity) {
                    Osmium::OSM::Way& way = this->prepare_way();

                    const OSMPBF::Way& pbf_way = group.ways(entity);

                    way.id(pbf_way.id());
                    if (pbf_way.has_info()) {
                        way.version(pbf_way.info().version())
                        .changeset(pbf_way.info().changeset())
                        .timestamp(pbf_way.info().timestamp() * m_date_factor)
                        .uid(pbf_way.info().uid())
                        .user(stringtable.s(pbf_way.info().user_sid()).data());
                        if (pbf_way.info().has_visible()) {
                            way.visible(pbf_way.info().visible());
                        }
                    }

                    Osmium::OSM::TagList& tags = way.tags();
                    for (int tag=0; tag < pbf_way.keys_size(); ++tag) {
                        tags.add(stringtable.s(pbf_way.keys(tag)).data(),
                                 stringtable.s(pbf_way.vals(tag)).data());
                    }

                    uint64_t ref = 0;
                    for (int i=0; i < pbf_way.refs_size(); ++i) {
                        ref += pbf_way.refs(i);
                        way.add_node(ref);
                    }

                    this->call_way_on_handler();
                }
            }

            // empty specialization to optimize the case where the relation() method on the handler is empty
            void parse_relation_group(const OSMPBF::PrimitiveGroup& /*group*/, const OSMPBF::StringTable& /*stringtable*/,
                                      void (Osmium::Handler::Base::*)(const shared_ptr<Osmium::OSM::Relation const>&) const) {
            }

            template <typename T>
            void parse_relation_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable, T) {
                int max_entity = group.relations_size();
                for (int entity=0; entity < max_entity; ++entity) {
                    Osmium::OSM::Relation& relation = this->prepare_relation();

                    const OSMPBF::Relation& pbf_relation = group.relations(entity);

                    relation.id(pbf_relation.id());
                    if (pbf_relation.has_info()) {
                        relation.version(pbf_relation.info().version())
                        .changeset(pbf_relation.info().changeset())
                        .timestamp(pbf_relation.info().timestamp() * m_date_factor)
                        .uid(pbf_relation.info().uid())
                        .user(stringtable.s(pbf_relation.info().user_sid()).data());
                        if (pbf_relation.info().has_visible()) {
                            relation.visible(pbf_relation.info().visible());
                        }
                    }

                    Osmium::OSM::TagList& tags = relation.tags();
                    for (int tag=0; tag < pbf_relation.keys_size(); ++tag) {
                        tags.add(stringtable.s(pbf_relation.keys(tag)).data(),
                                 stringtable.s(pbf_relation.vals(tag)).data());
                    }

                    uint64_t ref = 0;
                    for (int i=0; i < pbf_relation.types_size(); ++i) {
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
                        relation.add_member(type, ref, stringtable.s(pbf_relation.roles_sid(i)).data());
                    }

                    this->call_relation_on_handler();
                }
            }

            // empty specialization to optimize the case where the node() method on the handler is empty
            void parse_dense_node_group(const OSMPBF::PrimitiveGroup& /*group*/, const OSMPBF::StringTable& /*stringtable*/,
                                        void (Osmium::Handler::Base::*)(const shared_ptr<Osmium::OSM::Node const>&) const) {
            }

            template <typename T>
            void parse_dense_node_group(const OSMPBF::PrimitiveGroup& group, const OSMPBF::StringTable& stringtable, T) {
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
                    Osmium::OSM::Node& node = this->prepare_node();

                    last_dense_id += dense.id(entity);
                    node.id(last_dense_id);

                    if (dense.has_denseinfo()) {
                        last_dense_changeset += dense.denseinfo().changeset(entity);
                        last_dense_timestamp += dense.denseinfo().timestamp(entity);
                        last_dense_uid       += dense.denseinfo().uid(entity);
                        last_dense_user_sid  += dense.denseinfo().user_sid(entity);

                        node.version(dense.denseinfo().version(entity));
                        node.changeset(last_dense_changeset);
                        node.timestamp(last_dense_timestamp * m_date_factor);
                        node.uid(last_dense_uid);
                        node.user(stringtable.s(last_dense_user_sid).data());

                        if (dense.denseinfo().visible_size() > 0) {
                            node.visible(dense.denseinfo().visible(entity));
                        }
                    }

                    last_dense_latitude  += dense.lat(entity);
                    last_dense_longitude += dense.lon(entity);
                    node.position(Osmium::OSM::Position(
                                      (last_dense_longitude * m_pbf_primitive_block.granularity() + m_pbf_primitive_block.lon_offset()) / (OSMPBF::lonlat_resolution / Osmium::OSM::coordinate_precision),
                                      (last_dense_latitude  * m_pbf_primitive_block.granularity() + m_pbf_primitive_block.lat_offset()) / (OSMPBF::lonlat_resolution / Osmium::OSM::coordinate_precision)));

                    while (last_dense_tag < dense.keys_vals_size()) {
                        int tag_key_pos = dense.keys_vals(last_dense_tag);

                        if (tag_key_pos == 0) {
                            last_dense_tag++;
                            break;
                        }

                        Osmium::OSM::TagList& tags = node.tags();
                        tags.add(stringtable.s(tag_key_pos).data(),
                                 stringtable.s(dense.keys_vals(last_dense_tag+1)).data());

                        last_dense_tag += 2;
                    }

                    this->call_node_on_handler();
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
                int offset = 0;
                while (offset < static_cast<int>(sizeof(size_in_network_byte_order))) {
                    int nread = ::read(this->fd(), size_in_network_byte_order + offset, sizeof(size_in_network_byte_order) - offset);
                    if (nread < 0) {
                        throw std::runtime_error("read error");
                    } else if (nread == 0) {
                        return false; // EOF
                    }
                    offset += nread;
                }

                const int size = convert_from_network_byte_order(size_in_network_byte_order);
                if (size > OSMPBF::max_blob_header_size || size < 0) {
                    std::ostringstream errmsg;
                    errmsg << "BlobHeader size invalid:" << size;
                    throw std::runtime_error(errmsg.str());
                }

                offset = 0;
                while (offset < size) {
                    int nread = ::read(this->fd(), m_input_buffer + offset, size - offset);
                    if (nread < 1) {
                        throw std::runtime_error("failed to read BlobHeader");
                    }
                    offset += nread;
                }

                if (!m_pbf_blob_header.ParseFromArray(m_input_buffer, size)) {
                    throw std::runtime_error("failed to parse BlobHeader");
                }
                return true;
            }

            /**
            * Read a (possibly compressed) blob of data. If the blob is compressed, it is uncompressed.
            */
            array_t read_blob(const int size) {
                if (size < 0 || size > OSMPBF::max_uncompressed_blob_size) {
                    std::ostringstream errmsg;
                    errmsg << "invalid blob size: " << size;
                    throw std::runtime_error(errmsg.str());
                }
                int offset = 0;
                while (offset < size) {
                    int nread = ::read(this->fd(), m_input_buffer + offset, size - offset);
                    if (nread < 1) {
                        throw std::runtime_error("failed to read blob");
                    }
                    offset += nread;
                }
                if (!m_pbf_blob.ParseFromArray(m_input_buffer, size)) {
                    throw std::runtime_error("failed to parse blob");
                }

                if (m_pbf_blob.has_raw()) {
                    return array_t(m_pbf_blob.raw().data(), m_pbf_blob.raw().size());
                } else if (m_pbf_blob.has_zlib_data()) {
                    unsigned long raw_size = m_pbf_blob.raw_size();
                    assert(raw_size <= static_cast<unsigned long>(OSMPBF::max_uncompressed_blob_size));
                    if (uncompress(m_unpack_buffer, &raw_size, reinterpret_cast<const unsigned char*>(m_pbf_blob.zlib_data().data()), m_pbf_blob.zlib_data().size()) != Z_OK || m_pbf_blob.raw_size() != static_cast<long>(raw_size)) {
                        throw std::runtime_error("zlib error");
                    }
                    return array_t(m_unpack_buffer, raw_size);
                } else if (m_pbf_blob.has_lzma_data()) {
                    throw std::runtime_error("lzma blobs not implemented");
                } else {
                    throw std::runtime_error("Blob contains no data");
                }
            }

        }; // class PBF

    } // namespace Input

} // namespace Osmium

#endif // OSMIUM_INPUT_PBF_HPP
