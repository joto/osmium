#ifndef OSMIUM_SER_HANDLER_HPP
#define OSMIUM_SER_HANDLER_HPP

/*

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <boost/foreach.hpp>

#include <osmium/handler.hpp>
#include <osmium/ser/builder.hpp>

namespace Osmium {

    namespace Ser {

        template<class TBufferManager, class TIndexNode, class TIndexWay, class TIndexRelation, class TMapWayMember>
        class Handler : public Osmium::Handler::Base {

        public:

            Handler(TBufferManager& buffer_manager, TIndexNode& node_index, TIndexWay& way_index, TIndexRelation& relation_index, TMapWayMember& map_way_member, int data_fd = 1) :
                Osmium::Handler::Base(),
                m_buffer_manager(buffer_manager),
                m_buffer(buffer_manager.buffer()),
                m_offset(0),
                m_data_fd(data_fd),
                m_node_index(node_index),
                m_way_index(way_index),
                m_relation_index(relation_index),
                m_map_way_member(map_way_member) {
            }

            void init(Osmium::OSM::Meta&) const {
            }

            void flush_buffer() {
                ::write(m_data_fd, m_buffer.ptr(), m_buffer.committed());
                m_offset += m_buffer.clear();
            }

            void write_node(const shared_ptr<Osmium::OSM::Node const>& node) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::Node> builder(m_buffer);

                Osmium::Ser::Node& sn = builder.object();
                sn.id        = node->id();
                sn.version   = node->version();
                sn.timestamp = node->timestamp();
                sn.uid       = node->uid();
                sn.changeset = node->changeset();
                sn.pos       = node->position();

                builder.add_string(node->user());

                if (node->tags().size() > 0) {
                    builder.add_tags(node->tags());
                }

                assert(m_buffer.is_aligned());

                m_node_index.set(node->id(), m_offset + m_buffer.commit());
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                try {
                    write_node(node);
                } catch (std::range_error& e) {
                    flush_buffer();
                    write_node(node);
                }
            }

            void write_way(const shared_ptr<Osmium::OSM::Way const>& way) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::Way> builder(m_buffer);

                Osmium::Ser::Way& sn = builder.object();
                sn.id        = way->id();
                sn.version   = way->version();
                sn.timestamp = way->timestamp();
                sn.uid       = way->uid();
                sn.changeset = way->changeset();

                builder.add_string(way->user());
                builder.add_tags(way->tags());

                if (way->nodes()[0].position().defined()) {
                    builder.add_way_nodes_with_position(way->nodes());
                } else {
                    builder.add_way_nodes(way->nodes());
                }

                m_way_index.set(way->id(), m_offset + m_buffer.commit());
                BOOST_FOREACH(const Osmium::OSM::WayNode& wn, way->nodes()) {
                    m_map_way_member.set(wn.ref(), way->id());
                }
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& way) {
                try {
                    write_way(way);
                } catch (std::range_error& e) {
                    flush_buffer();
                    write_way(way);
                }
            }

            void write_relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                Osmium::Ser::ObjectBuilder<Osmium::Ser::Relation> builder(m_buffer);

                Osmium::Ser::Relation& sn = builder.object();
                sn.id        = relation->id();
                sn.version   = relation->version();
                sn.timestamp = relation->timestamp();
                sn.uid       = relation->uid();
                sn.changeset = relation->changeset();

                builder.add_string(relation->user());
                builder.add_tags(relation->tags());
                builder.add_members(relation->members());

                m_relation_index.set(relation->id(), m_offset + m_buffer.commit());
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                try {
                    write_relation(relation);
                } catch (std::range_error& e) {
                    flush_buffer();
                    write_relation(relation);
                }
            }

            void final() {
                flush_buffer();
            }

        private:

            TBufferManager& m_buffer_manager;
            Osmium::Ser::Buffer& m_buffer;
            size_t m_offset;
            int m_data_fd;
            TIndexNode& m_node_index;
            TIndexWay& m_way_index;
            TIndexRelation& m_relation_index;
            TMapWayMember& m_map_way_member;

        }; // class Handler

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_HANDLER_HPP
