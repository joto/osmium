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

    class PBFParser {

        static const int NANO = 1000 * 1000 * 1000;
        static const int MAX_BLOB_HEADER_SIZE = 64 * 1024;
        static const int MAX_BLOB_SIZE = 32 * 1024 * 1024;

        enum Mode {
            ModeNode, ModeWay, ModeRelation, ModeDense
        };

        char buffer[MAX_BLOB_SIZE];
        char unpack_buffer[MAX_BLOB_SIZE];
        int fd;
        std::ostringstream errmsg;
        struct callbacks *callbacks;

        typedef struct {
            const void *data;
            size_t size;
        } array_t;

        int groups_with_nodes;
        int groups_with_ways;
        int groups_with_relations;

        OSMPBF::BlobHeader m_blobHeader;

        OSMPBF::HeaderBlock m_headerBlock;
        OSMPBF::PrimitiveBlock m_primitiveBlock;

        std::map< std::string, int > m_nodeTags;
        std::map< std::string, int > m_wayTags;
        std::map< std::string, int > m_relationTags;

        std::vector< int > m_nodeTagIDs;
        std::vector< int > m_wayTagIDs;
        std::vector< int > m_relationTagIDs;

        int64_t m_lastDenseID;
        int64_t m_lastDenseLatitude;
        int64_t m_lastDenseLongitude;
        int64_t m_lastDenseUID;
        int64_t m_lastDenseUserSID;
        int64_t m_lastDenseChangeset;
        int64_t m_lastDenseTimestamp;
        int m_lastDenseTag;

      public:

        PBFParser(int in_fd, struct callbacks *cb) : fd(in_fd), callbacks(cb) {
            GOOGLE_PROTOBUF_VERIFY_VERSION;
            groups_with_nodes     = 0;
            groups_with_ways      = 0;
            groups_with_relations = 0;
        }

        ~PBFParser () {
            google::protobuf::ShutdownProtobufLibrary();
        }

        void parse(Osmium::OSM::Node *in_node, Osmium::OSM::Way *in_way, Osmium::OSM::Relation *in_relation) {
            while (read_blob_header()) {
                array_t a = read_blob(m_blobHeader.datasize());

                if (m_blobHeader.type() == "OSMData") {
                    if (debug) {
                        std::cerr << "Got blob of type OSMData" << std::endl;
                    }
                    if (!m_primitiveBlock.ParseFromArray(a.data, a.size)) {
                        throw std::runtime_error("Failed to parse PrimitiveBlock.");
                    }
                    for (int i=0; i < m_primitiveBlock.primitivegroup_size(); i++) {
                        parse_group(m_primitiveBlock.primitivegroup(i), in_node, in_way, in_relation);
                    }
                } else if (m_blobHeader.type() == "OSMHeader") {
                    if (debug) {
                        std::cerr << "Got blob of type OSMHeader" << std::endl;
                    }
                    if (!m_headerBlock.ParseFromArray(a.data, a.size)) {
                        throw std::runtime_error("Failed to parse HeaderBlock.");
                    }

                    for (int i=0; i < m_headerBlock.required_features_size(); i++) {
                        const std::string& feature = m_headerBlock.required_features(i);

                        if ((feature != "OsmSchema-V0.6") && (feature != "DenseNodes")) {
                            errmsg << "Required feature not supported: " << feature;
                            throw std::runtime_error(errmsg.str());
                        }
                    }
                } else {
                    if (debug) {
                        std::cerr << "Ignoring unknown blob type (" << m_blobHeader.type().data() << ")." << std::endl;
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

        void parse_group(const OSMPBF::PrimitiveGroup& group, Osmium::OSM::Node *in_node, Osmium::OSM::Way *in_way, Osmium::OSM::Relation *in_relation) {
            int max_entity;

            if (group.has_dense())  {
                if (groups_with_nodes == 0) {
                    if (callbacks->before_nodes) { callbacks->before_nodes(); }
                }
                groups_with_nodes++;
                max_entity = group.dense().id_size();
                m_lastDenseID = 0;
                m_lastDenseUID = 0;
                m_lastDenseUserSID = 0;
                m_lastDenseChangeset = 0;
                m_lastDenseTimestamp = 0;
                m_lastDenseLatitude = 0;
                m_lastDenseLongitude = 0;
                m_lastDenseTag = 0;
                assert( group.dense().id_size() != 0 );
                for (int m_currentEntity = 0; m_currentEntity < max_entity; m_currentEntity++) {
                    parseDense(group, m_currentEntity, in_node);
                    if (callbacks->node) { callbacks->node(in_node); }
                }
            } else if (group.ways_size() != 0) {
                if (groups_with_ways == 0) {
                    if (groups_with_nodes > 0) {
                        if (callbacks->after_nodes) { callbacks->after_nodes(); }
                    }
                    if (callbacks->before_ways) { callbacks->before_ways(); }
                }
                groups_with_ways++;
                max_entity = group.ways_size();
                for (int m_currentEntity = 0; m_currentEntity < max_entity; m_currentEntity++) {
                    parseWay(group, m_currentEntity, in_way);
                    if (callbacks->way) { callbacks->way(in_way); }
                }
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
                max_entity = group.relations_size();
                for (int m_currentEntity = 0; m_currentEntity < max_entity; m_currentEntity++) {
                    parseRelation(group, m_currentEntity, in_relation);
                    if (callbacks->relation) { callbacks->relation(in_relation); }
                }
            } else if (group.nodes_size() != 0) {
                if (groups_with_nodes == 0) {
                    if (callbacks->before_nodes) { callbacks->before_nodes(); }
                }
                groups_with_nodes++;
                max_entity = group.nodes_size();
                for (int m_currentEntity = 0; m_currentEntity < max_entity; m_currentEntity++) {
                    parseNode(group, m_currentEntity, in_node);
                    if (callbacks->node) { callbacks->node(in_node); }
                }
            } else {
                throw std::runtime_error("Group of unknown type.");
            }
        }

        int convertNetworkByteOrder( char data[4] ) {
            return ( ( ( unsigned ) data[0] ) << 24 ) | ( ( ( unsigned ) data[1] ) << 16 ) | ( ( ( unsigned ) data[2] ) << 8 ) | ( unsigned ) data[3];
        }

        void parseNode(const OSMPBF::PrimitiveGroup& group, int entity, Osmium::OSM::Node* node) {
            node->reset();

            const OSMPBF::Node& inputNode = group.nodes(entity);
            node->id = inputNode.id();
            node->version = inputNode.info().version();
            node->uid = inputNode.info().uid();
            if (! memccpy(node->user, m_primitiveBlock.stringtable().s(inputNode.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                throw std::length_error("user name too long");
            }
            node->changeset = inputNode.info().changeset();
            node->timestamp = inputNode.info().timestamp();
            node->set_coordinates(( ( double ) inputNode.lon() * m_primitiveBlock.granularity() + m_primitiveBlock.lon_offset() ) / NANO,
                                  ( ( double ) inputNode.lat() * m_primitiveBlock.granularity() + m_primitiveBlock.lat_offset() ) / NANO);
            for (int tag=0; tag < inputNode.keys_size(); tag++) {
                node->add_tag(m_primitiveBlock.stringtable().s( inputNode.keys( tag ) ).data(),
                              m_primitiveBlock.stringtable().s( inputNode.vals( tag ) ).data() );
            }
        }

        void parseWay(const OSMPBF::PrimitiveGroup& group, int entity, Osmium::OSM::Way* way) {
            way->reset();

            const OSMPBF::Way& inputWay = group.ways(entity);
            way->id = inputWay.id();
            way->version = inputWay.info().version();
            way->uid = inputWay.info().uid();
            if (! memccpy(way->user, m_primitiveBlock.stringtable().s(inputWay.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                throw std::length_error("user name too long");
            }
            way->changeset = inputWay.info().changeset();
            way->timestamp = inputWay.info().timestamp();
            for (int tag=0; tag < inputWay.keys_size(); tag++) {
                way->add_tag( m_primitiveBlock.stringtable().s( inputWay.keys( tag ) ).data(),
                            m_primitiveBlock.stringtable().s( inputWay.vals( tag ) ).data() );
            }

            uint64_t lastRef = 0;
            for (int i=0; i < inputWay.refs_size(); i++) {
                lastRef += inputWay.refs( i );
                way->add_node( lastRef );
            }
        }

        void parseRelation(const OSMPBF::PrimitiveGroup& group, int entity, Osmium::OSM::Relation* relation) {
            relation->reset();

            const OSMPBF::Relation& inputRelation = group.relations(entity);
            relation->id = inputRelation.id();
            relation->version = inputRelation.info().version();
            relation->uid = inputRelation.info().uid();
            if (! memccpy(relation->user, m_primitiveBlock.stringtable().s(inputRelation.info().user_sid()).data(), 0, Osmium::OSM::Object::max_length_username)) {
                throw std::length_error("user name too long");
            }
            relation->changeset = inputRelation.info().changeset();
            relation->timestamp = inputRelation.info().timestamp();
            for (int tag=0; tag < inputRelation.keys_size(); tag++) {
                relation->add_tag( m_primitiveBlock.stringtable().s( inputRelation.keys(tag) ).data(),
                                   m_primitiveBlock.stringtable().s( inputRelation.vals(tag) ).data() );
            }

            uint64_t lastRef = 0;
            for (int i=0; i < inputRelation.types_size(); i++) {
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
        }

        void parseDense(const OSMPBF::PrimitiveGroup& group, int entity, Osmium::OSM::Node* node) {
            node->reset();

            const OSMPBF::DenseNodes& dense = group.dense();
            m_lastDenseID += dense.id(entity);
            m_lastDenseLatitude += dense.lat(entity);
            m_lastDenseLongitude += dense.lon(entity);
            m_lastDenseUID += dense.denseinfo().uid(entity);
            m_lastDenseUserSID += dense.denseinfo().user_sid(entity);
            m_lastDenseChangeset += dense.denseinfo().changeset(entity);
            m_lastDenseTimestamp += dense.denseinfo().timestamp(entity);
            node->id = m_lastDenseID;
            node->version = dense.denseinfo().version(entity);
            node->uid = m_lastDenseUID;
            if (! memccpy(node->user, m_primitiveBlock.stringtable().s( m_lastDenseUserSID ).data(), 0, Osmium::OSM::Object::max_length_username)) {
                throw std::length_error("user name too long");
            }
            node->changeset = m_lastDenseChangeset;
            node->timestamp = m_lastDenseTimestamp;
            node->set_coordinates(( ( double ) m_lastDenseLongitude * m_primitiveBlock.granularity() + m_primitiveBlock.lon_offset() ) / NANO,
                                  ( ( double ) m_lastDenseLatitude * m_primitiveBlock.granularity() + m_primitiveBlock.lat_offset() ) / NANO);

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
        }

        bool read_blob_header() {
            char size_in_network_byte_order[4];
            ssize_t bytes_read = read(fd, size_in_network_byte_order, sizeof(size_in_network_byte_order));
            if (bytes_read != sizeof(size_in_network_byte_order)) {
                if (bytes_read == 0) {
                    return false; // EOF
                }
                throw std::runtime_error("read error");
            }

            int size = convertNetworkByteOrder(size_in_network_byte_order);
            if (size > MAX_BLOB_HEADER_SIZE || size < 0) {
                errmsg << "BlobHeader size invalid:" << size;
                throw std::runtime_error(errmsg.str());
            }

            if (read(fd, buffer, size) != size) {
                throw std::runtime_error("failed to read BlobHeader");
            }

            if (!m_blobHeader.ParseFromArray(buffer, size)) {
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
                unpackZlib(blob.zlib_data().data(), blob.zlib_data().size(), blob.raw_size());
                return { unpack_buffer, blob.raw_size() };
            } else if (blob.has_lzma_data()) {
                throw std::runtime_error("lzma blobs not implemented");
            } else {
                throw std::runtime_error("Blob contains no data");
            }
        }

        void unpackZlib(const char *data, size_t size, size_t raw_size) {
            z_stream compressedStream;

            compressedStream.next_in   = (unsigned char*) data;
            compressedStream.avail_in  = size;
            compressedStream.next_out  = (unsigned char*) unpack_buffer;
            compressedStream.avail_out = raw_size;
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

    }; // class PBFParser

} // namespace Osmium

#endif // PBFPARSER_HPP
