#ifndef OSMIUM_HANDLER_RANGE_FROM_HISTORY_HPP
#define OSMIUM_HANDLER_RANGE_FROM_HISTORY_HPP

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
         * Handler to extract only objects valid in a given timestamp range.
         * Use from==to to extract objects valid at a certain time.
         *
         * Needs the endtime() to be set in objects, so you have to stack it
         * after the EndTime() handler.
         */
        template <class THandler>
        class RangeFromHistory : public Base {

        public:

            RangeFromHistory(THandler* handler, time_t from, time_t to) : Base(), m_handler(handler), m_from(from), m_to(to) {
            }

            void init(Osmium::OSM::Meta& meta) {
                m_handler->init(meta);
            }

            void before_nodes() {
                m_handler->before_nodes();
            }

            void node(const shared_ptr<Osmium::OSM::Node>& node) {
                if ((node->endtime() == 0 || node->endtime() >= m_from) && node->timestamp() <= m_to) {
                    m_handler->node(node);
                }
            }

            void after_nodes() {
                m_handler->after_nodes();
            }

            void before_ways() {
                m_handler->before_ways();
            }

            void way(const shared_ptr<Osmium::OSM::Way>& way) {
                if ((way->endtime() == 0 || way->endtime() >= m_from) && way->timestamp() <= m_to) {
                    m_handler->way(way);
                }
            }

            void after_ways() {
                m_handler->after_ways();
            }

            void before_relations() {
                m_handler->before_relations();
            }

            void relation(const shared_ptr<Osmium::OSM::Relation>& relation) {
                if ((relation->endtime() == 0 || relation->endtime() >= m_from) && relation->timestamp() <= m_to) {
                    m_handler->relation(relation);
                }
            }

            void after_relations() {
                m_handler->after_relations();
            }

            void final() {
                m_handler->final();
            }

        private:

            THandler* m_handler;

            const time_t m_from;
            const time_t m_to;

        }; // class RangeFromHistory

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_RANGE_FROM_HISTORY_HPP
