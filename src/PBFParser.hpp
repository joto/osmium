/*

The code in this file is based on the MoNav code, see
http://wiki.openstreetmap.org/wiki/MoNav .

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
along with MoNav.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PBFPARSER_HPP
#define PBFPARSER_HPP

#include "protobuf/fileformat.pb.h"
#include "protobuf/osmformat.pb.h"
#include <string>
#include <zlib.h>

namespace Osmium {

    class PBFParser {

      protected:

        static const int NANO = 1000 * 1000 * 1000;
        static const int MAX_BLOCK_HEADER_SIZE = 64 * 1024;
        static const int MAX_BLOB_SIZE = 32 * 1024 * 1024;

        enum Mode {
            ModeNode, ModeWay, ModeRelation, ModeDense
        };

        char buffer[MAX_BLOB_SIZE];
        int fd;
        std::ostringstream errmsg;
        struct callbacks *callbacks;

      public:

        PBFParser(int in_fd, struct callbacks *cb) {
            GOOGLE_PROTOBUF_VERIFY_VERSION;
            fd = in_fd;
            callbacks = cb;
            last_mode = ModeNode;

            readBlockHeader();

            if (m_blockHeader.type() != "OSMHeader") {
                errmsg << "OSMHeader missing, found " << m_blockHeader.type().data() << " instead";
                throw std::runtime_error(errmsg.str());
            }

            if (!m_headerBlock.ParseFromArray(buffer, readBlob())) {
                throw std::runtime_error("failed to parse HeaderBlock");
            }

            for (int i = 0; i < m_headerBlock.required_features_size(); i++) {
                const std::string& feature = m_headerBlock.required_features( i );
                bool supported = false;
                if (feature == "OsmSchema-V0.6") { // XXX
                    supported = true;
                } else if (feature == "DenseNodes") {
                    supported = true;
                }

                if (!supported) {
                    errmsg << "required feature not supported:" << feature.data();
                    throw std::runtime_error(errmsg.str());
                }
            }

            m_loadBlock = true;
        }

        void parse(Osmium::OSM::Node *in_node, Osmium::OSM::Way *in_way, Osmium::OSM::Relation *in_relation) {
            if (callbacks->before_nodes) { callbacks->before_nodes(); }
            while (1) {
                if (m_loadBlock) {
                    if (!readNextBlock()) {
                        if (callbacks->after_relations) { callbacks->after_relations(); }
                        return; // EOF
                    }
                    loadBlock();
                    loadGroup();
                }

                if (last_mode != m_mode) {
                    switch (m_mode) {
                        case ModeNode:
                            break;
                        case ModeDense:
                            break;
                        case ModeWay:
                            if (callbacks->after_nodes) { callbacks->after_nodes(); }
                            if (callbacks->before_ways) { callbacks->before_ways(); }
                            break;
                        case ModeRelation:
                            if (callbacks->after_ways) { callbacks->after_ways(); }
                            if (callbacks->before_relations) { callbacks->before_relations(); }
                            break;
                    }
                    last_mode = m_mode;
                }

                switch (m_mode) {
                    case ModeNode:
                        parseNode(in_node);
                        if (callbacks->node) { callbacks->node(in_node); }
                        break;
                    case ModeWay:
                        parseWay(in_way);
                        if (callbacks->way) { callbacks->way(in_way); }
                        break;
                    case ModeRelation:
                        parseRelation(in_relation);
                        if (callbacks->relation) { callbacks->relation(in_relation); }
                        break;
                    case ModeDense:
                        parseDense(in_node);
                        if (callbacks->node) { callbacks->node(in_node); }
                        break;
                }

            }
        }

        ~PBFParser () {
            google::protobuf::ShutdownProtobufLibrary();
        }

    protected:

        void nextEntity(int max) {
            m_currentEntity++;
            if (m_currentEntity >= max) {
                m_currentEntity = 0;
                m_currentGroup++;
                if (m_currentGroup >= m_primitiveBlock.primitivegroup_size()) {
                    m_loadBlock = true;
                } else {
                    loadGroup();
                }
            }
        }

        int convertNetworkByteOrder( char data[4] ) {
            return ( ( ( unsigned ) data[0] ) << 24 ) | ( ( ( unsigned ) data[1] ) << 16 ) | ( ( ( unsigned ) data[2] ) << 8 ) | ( unsigned ) data[3];
        }

        void parseNode( Osmium::OSM::Node* node ) {
            node->reset();

            const OSMPBF::Node& inputNode = m_primitiveBlock.primitivegroup( m_currentGroup ).nodes( m_currentEntity );
            node->id = inputNode.id();
            node->version = inputNode.info().version();
            node->uid = inputNode.info().uid();
            if (! memccpy(node->user, m_primitiveBlock.stringtable().s(inputNode.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                throw std::length_error("user name too long");
            }
            node->changeset = inputNode.info().changeset();
            node->timestamp = inputNode.info().timestamp();
            node->geom.point.x = ( ( double ) inputNode.lon() * m_primitiveBlock.granularity() + m_primitiveBlock.lon_offset() ) / NANO;
            node->geom.point.y = ( ( double ) inputNode.lat() * m_primitiveBlock.granularity() + m_primitiveBlock.lat_offset() ) / NANO;
            for ( int tag = 0; tag < inputNode.keys_size(); tag++ ) {
                node->add_tag(m_primitiveBlock.stringtable().s( inputNode.keys( tag ) ).data(),
                              m_primitiveBlock.stringtable().s( inputNode.vals( tag ) ).data() );
            }

            nextEntity(m_primitiveBlock.primitivegroup(m_currentGroup).nodes_size());
        }

        void parseWay( Osmium::OSM::Way* way ) {
            way->reset();

            const OSMPBF::Way& inputWay = m_primitiveBlock.primitivegroup( m_currentGroup ).ways( m_currentEntity );
            way->id = inputWay.id();
            way->version = inputWay.info().version();
            way->uid = inputWay.info().uid();
            if (! memccpy(way->user, m_primitiveBlock.stringtable().s(inputWay.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                throw std::length_error("user name too long");
            }
            way->changeset = inputWay.info().changeset();
            way->timestamp = inputWay.info().timestamp();
            for (int tag = 0; tag < inputWay.keys_size(); tag++) {
                way->add_tag( m_primitiveBlock.stringtable().s( inputWay.keys( tag ) ).data(),
                            m_primitiveBlock.stringtable().s( inputWay.vals( tag ) ).data() );
            }

            uint64_t lastRef = 0;
            for (int i = 0; i < inputWay.refs_size(); i++) {
                lastRef += inputWay.refs( i );
                way->add_node( lastRef );
            }

            nextEntity(m_primitiveBlock.primitivegroup(m_currentGroup).ways_size());
        }

        void parseRelation( Osmium::OSM::Relation* relation ) {
            relation->reset();

            const OSMPBF::Relation& inputRelation = m_primitiveBlock.primitivegroup( m_currentGroup ).relations( m_currentEntity );
            relation->id = inputRelation.id();
            relation->version = inputRelation.info().version();
            relation->uid = inputRelation.info().uid();
            if (! memccpy(relation->user, m_primitiveBlock.stringtable().s(inputRelation.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                throw std::length_error("user name too long");
            }
            relation->changeset = inputRelation.info().changeset();
            relation->timestamp = inputRelation.info().timestamp();
            for (int tag = 0; tag < inputRelation.keys_size(); tag++) {
                relation->add_tag( m_primitiveBlock.stringtable().s( inputRelation.keys(tag) ).data(),
                                   m_primitiveBlock.stringtable().s( inputRelation.vals(tag) ).data() );
            }

            uint64_t lastRef = 0;
            for (int i = 0; i < inputRelation.types_size(); i++) {
                char type = 'x';
                switch ( inputRelation.types(i) ) {
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
                relation->add_member(type, lastRef, m_primitiveBlock.stringtable().s( inputRelation.roles_sid( i ) ).data());
            }

            nextEntity(m_primitiveBlock.primitivegroup(m_currentGroup).relations_size());
        }

        void parseDense( Osmium::OSM::Node* node ) {
            node->reset();

            const OSMPBF::DenseNodes& dense = m_primitiveBlock.primitivegroup( m_currentGroup ).dense();
            m_lastDenseID += dense.id( m_currentEntity );
            m_lastDenseLatitude += dense.lat( m_currentEntity );
            m_lastDenseLongitude += dense.lon( m_currentEntity );
            m_lastDenseUID += dense.denseinfo().uid( m_currentEntity );
            m_lastDenseUserSID += dense.denseinfo().user_sid( m_currentEntity );
            m_lastDenseChangeset += dense.denseinfo().changeset( m_currentEntity );
            m_lastDenseTimestamp += dense.denseinfo().timestamp( m_currentEntity );
            node->id = m_lastDenseID;
            node->version = dense.denseinfo().version( m_currentEntity );
            node->uid = m_lastDenseUID;
            if (! memccpy(node->user, m_primitiveBlock.stringtable().s( m_lastDenseUserSID ).data(), 0, Osmium::OSM::Object::max_length_username)) {
                throw std::length_error("user name too long");
            }
            node->changeset = m_lastDenseChangeset;
            node->timestamp = m_lastDenseTimestamp;
            node->geom.point.x = ( ( double ) m_lastDenseLongitude * m_primitiveBlock.granularity() + m_primitiveBlock.lon_offset() ) / NANO;
            node->geom.point.y = ( ( double ) m_lastDenseLatitude * m_primitiveBlock.granularity() + m_primitiveBlock.lat_offset() ) / NANO;

            while (m_lastDenseTag < dense.keys_vals_size()) {
                int tagValue = dense.keys_vals( m_lastDenseTag );

                if (tagValue == 0) {
                    m_lastDenseTag++;
                    break;
                }

                node->add_tag( m_primitiveBlock.stringtable().s( dense.keys_vals( m_lastDenseTag     ) ).data(),
                               m_primitiveBlock.stringtable().s( dense.keys_vals( m_lastDenseTag + 1 ) ).data() );

                m_lastDenseTag += 2;
            }

            nextEntity(dense.id_size());
        }

        void loadGroup() {
            const OSMPBF::PrimitiveGroup& group = m_primitiveBlock.primitivegroup( m_currentGroup );
            if (group.nodes_size() != 0) {
                m_mode = ModeNode;
            } else if (group.ways_size() != 0) {
                m_mode = ModeWay;
            } else if (group.relations_size() != 0) {
                m_mode = ModeRelation;
            } else if (group.has_dense())  {
                m_mode = ModeDense;
                m_lastDenseID = 0;
                m_lastDenseUID = 0;
                m_lastDenseUserSID = 0;
                m_lastDenseChangeset = 0;
                m_lastDenseTimestamp = 0;
                m_lastDenseLatitude = 0;
                m_lastDenseLongitude = 0;
                m_lastDenseTag = 0;
                assert( group.dense().id_size() != 0 );
            } else {
                throw std::runtime_error("entity of unknown type");
            }
        }

        void loadBlock() {
            m_loadBlock = false;
            m_currentGroup = 0;
            m_currentEntity = 0;
        }

        bool readNextBlock() {
            if (!readBlockHeader()) {
                return false; // EOF
            }

            if (m_blockHeader.type() != "OSMData") {
                errmsg << "invalid block type, found " << m_blockHeader.type().data() << " instead of OSMData";
                throw std::runtime_error(errmsg.str());
            }

            if (!m_primitiveBlock.ParseFromArray(buffer, readBlob())) {
                errmsg << "failed to parse PrimitiveBlock";
                throw std::runtime_error(errmsg.str());
            }
            return true;
        }

        bool readBlockHeader() {
            char size_in_network_byte_order[4];
            ssize_t bytes_read = read(fd, size_in_network_byte_order, sizeof(size_in_network_byte_order));
            if (bytes_read != sizeof(size_in_network_byte_order)) {
                if (bytes_read == 0) {
                    return false; // EOF
                }
                throw std::runtime_error("read error");
            }

            int size = convertNetworkByteOrder(size_in_network_byte_order);
            if (size > MAX_BLOCK_HEADER_SIZE || size < 0) {
                errmsg << "BlockHeader size invalid:" << size;
                throw std::runtime_error(errmsg.str());
            }

            if (read(fd, buffer, size) != size) {
                throw std::runtime_error("failed to read BlockHeader");
            }

            if (!m_blockHeader.ParseFromArray(buffer, size)) {
                throw std::runtime_error("failed to parse BlockHeader");
            }
            return true;
        }

        int readBlob() {
            int size = m_blockHeader.datasize();
            if (size < 0 || size > MAX_BLOB_SIZE) {
                errmsg << "invalid blob size: " << size;
                throw std::runtime_error(errmsg.str());
            }
            if (read(fd, buffer, size) != size) {
                throw std::runtime_error("failed to read blob");
            }
            if (!m_blob.ParseFromArray(buffer, size)) {
                throw std::runtime_error("failed to parse blob");
            }

            if (m_blob.has_raw()) {
                // this should probably be done without the memcpy, but why bother of nobody uses uncompressed blobs anyway
                memcpy(buffer, m_blob.raw().data(), m_blob.raw_size());
            } else if (m_blob.has_zlib_data()) {
                unpackZlib();
            } else if (m_blob.has_lzma_data()) {
                throw std::runtime_error("lzma blobs not implemented");
            } else {
                throw std::runtime_error("Blob contains no data");
            }
            return m_blob.raw_size();
        }

        void unpackZlib() {
            z_stream compressedStream;

            compressedStream.next_in   = (unsigned char*) m_blob.zlib_data().data();
            compressedStream.avail_in  = m_blob.zlib_data().size();
            compressedStream.next_out  = (unsigned char*) buffer;
            compressedStream.avail_out = m_blob.raw_size();
            compressedStream.zalloc    = Z_NULL;
            compressedStream.zfree     = Z_NULL;
            compressedStream.opaque    = Z_NULL;

            if (inflateInit(&compressedStream) != Z_OK) {
                throw std::runtime_error("failed to init zlib stream");
            }
            if (inflate(&compressedStream, Z_FINISH) != Z_STREAM_END) {
                throw std::runtime_error("failed to inflate zlib stream");
            }
            if (inflateEnd(&compressedStream) != Z_OK) {
                throw std::runtime_error("failed to deinit zlib stream");
            }
        }

        OSMPBF::BlockHeader m_blockHeader;
        OSMPBF::Blob m_blob;

        OSMPBF::HeaderBlock m_headerBlock;
        OSMPBF::PrimitiveBlock m_primitiveBlock;

        int m_currentGroup;
        int m_currentEntity;
        bool m_loadBlock;

        Mode m_mode;
        Mode last_mode;

        std::map< std::string, int > m_nodeTags;
        std::map< std::string, int > m_wayTags;
        std::map< std::string, int > m_relationTags;

        std::vector< int > m_nodeTagIDs;
        std::vector< int > m_wayTagIDs;
        std::vector< int > m_relationTagIDs;

        uint64_t m_lastDenseID;
        uint64_t m_lastDenseLatitude;
        uint64_t m_lastDenseLongitude;
        uint64_t m_lastDenseUID;
        uint64_t m_lastDenseUserSID;
        uint64_t m_lastDenseChangeset;
        uint64_t m_lastDenseTimestamp;
        int m_lastDenseTag;

    };
}

#endif // PBPARSER_HPP
