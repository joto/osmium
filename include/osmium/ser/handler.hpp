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

        template<class TBufferManager>
        class Handler : public Osmium::Handler::Base {

        public:

            Handler(TBufferManager& buffer_manager) : Osmium::Handler::Base(), m_buffer_manager(buffer_manager), m_buffer(buffer_manager.buffer()) {
            }

            void init(Osmium::OSM::Meta&) const {
            }

            void flush_buffer() const {
                std::cout.write(m_buffer.ptr(), m_buffer.committed());
                m_buffer.clear();
            }

            void write_node(const shared_ptr<Osmium::OSM::Node const>& node) const {
                Osmium::Ser::NodeBuilder builder(m_buffer);

                Osmium::Ser::Node& sn = builder.node();
                sn.offset    = 0;
                sn.id        = node->id();
                sn.version   = node->version();
                sn.timestamp = node->timestamp();
                sn.uid       = node->uid();
                sn.changeset = node->changeset();
                sn.pos       = node->position();

                Osmium::Ser::UserNameBuilder username(m_buffer, &builder, node->user());
                Osmium::Ser::TagListBuilder tags(m_buffer, &builder);
                BOOST_FOREACH(const Osmium::OSM::Tag& tag, node->tags()) {
                    tags.add_tag(tag.key(), tag.value());
                }
                tags.done();

                m_buffer.commit();
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) const {
                try {
                    write_node(node);
                } catch (std::range_error& e) {
                    flush_buffer();
                    write_node(node);
                }
            }

            void write_way(const shared_ptr<Osmium::OSM::Way const>& way) const {
                Osmium::Ser::WayBuilder builder(m_buffer);

                Osmium::Ser::Way& sn = builder.way();
                sn.offset    = 0;
                sn.id        = way->id();
                sn.version   = way->version();
                sn.timestamp = way->timestamp();
                sn.uid       = way->uid();
                sn.changeset = way->changeset();

                Osmium::Ser::UserNameBuilder username(m_buffer, &builder, way->user());
                Osmium::Ser::TagListBuilder tags(m_buffer, &builder);
                BOOST_FOREACH(const Osmium::OSM::Tag& tag, way->tags()) {
                    tags.add_tag(tag.key(), tag.value());
                }
                tags.done();
                Osmium::Ser::NodeListBuilder nodes(m_buffer, &builder);
                BOOST_FOREACH(const Osmium::OSM::WayNode& way_node, way->nodes()) {
                    nodes.add_node(way_node.ref());
                }
                nodes.done();

                m_buffer.commit();
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& way) const {
                try {
                    write_way(way);
                } catch (std::range_error& e) {
                    flush_buffer();
                    write_way(way);
                }
            }

            void write_relation(const shared_ptr<Osmium::OSM::Relation const>& relation) const {
                Osmium::Ser::RelationBuilder builder(m_buffer);

                Osmium::Ser::Relation& sn = builder.relation();
                sn.offset    = 0;
                sn.id        = relation->id();
                sn.version   = relation->version();
                sn.timestamp = relation->timestamp();
                sn.uid       = relation->uid();
                sn.changeset = relation->changeset();

                Osmium::Ser::UserNameBuilder username(m_buffer, &builder, relation->user());
                Osmium::Ser::TagListBuilder tags(m_buffer, &builder);
                BOOST_FOREACH(const Osmium::OSM::Tag& tag, relation->tags()) {
                    tags.add_tag(tag.key(), tag.value());
                }
                tags.done();

                m_buffer.commit();
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) const {
                try {
                    write_relation(relation);
                } catch (std::range_error& e) {
                    flush_buffer();
                    write_relation(relation);
                }
            }

        private:

            TBufferManager& m_buffer_manager;
            Osmium::Ser::Buffer& m_buffer;

        }; // class Handler

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_HANDLER_HPP
