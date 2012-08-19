#ifndef OSMIUM_HANDLER_ENDTIME_HPP
#define OSMIUM_HANDLER_ENDTIME_HPP

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

#include <osmium/handler.hpp>

namespace Osmium {

    namespace Handler {

        /**
         * This handler adds the "end time" to each object by taking the start
         * time (timestamp) of the next object with the same version number.
         * It makes only sense to use this handler with history files.
         */
        template <class THandler>
        class EndTime : public Base {

        public:

            EndTime(THandler& handler) :
                Base(),
                m_handler(handler),
                m_last_node(),
                m_last_way(),
                m_last_relation() {
            }

            void init(Osmium::OSM::Meta& meta) {
                m_handler.init(meta);
            }

            void before_nodes() {
                m_handler.before_nodes();
            }

            void node(const shared_ptr<Osmium::OSM::Node>& node) {
                if (m_last_node) {
                    if (node->id() == m_last_node->id()) {
                        m_last_node->endtime(node->timestamp());
                    }
                    m_handler.node(m_last_node);
                }
                m_last_node = node;
            }

            void after_nodes() {
                if (m_last_node) {
                    m_handler.node(m_last_node);
                    m_last_node.reset();
                }
                m_handler.after_nodes();
            }

            void before_ways() {
                m_handler.before_ways();
            }

            void way(const shared_ptr<Osmium::OSM::Way>& way) {
                if (m_last_way) {
                    if (way->id() == m_last_way->id()) {
                        m_last_way->endtime(way->timestamp());
                    }
                    m_handler.way(m_last_way);
                }
                m_last_way = way;
            }

            void after_ways() {
                if (m_last_way) {
                    m_handler.way(m_last_way);
                    m_last_way.reset();
                }
                m_handler.after_ways();
            }

            void before_relations() {
                m_handler.before_relations();
            }

            void relation(const shared_ptr<Osmium::OSM::Relation>& relation) {
                if (m_last_relation) {
                    if (relation->id() == m_last_relation->id()) {
                        m_last_relation->endtime(relation->timestamp());
                    }
                    m_handler.relation(m_last_relation);
                }
                m_last_relation = relation;
            }

            void after_relations() {
                if (m_last_relation) {
                    m_handler.relation(m_last_relation);
                    m_last_relation.reset();
                }
                m_handler.after_relations();
            }

            void final() {
                m_handler.final();
            }

        private:

            THandler& m_handler;

            shared_ptr<Osmium::OSM::Node>     m_last_node;
            shared_ptr<Osmium::OSM::Way>      m_last_way;
            shared_ptr<Osmium::OSM::Relation> m_last_relation;

        }; // class EndTime

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_ENDTIME_HPP
