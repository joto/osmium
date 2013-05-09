#ifndef OSMIUM_HANDLER_PROGRESS_HPP
#define OSMIUM_HANDLER_PROGRESS_HPP

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

#include <unistd.h>
#include <sys/time.h>
#include <iostream>

#include <osmium/handler.hpp>

namespace Osmium {

    namespace Handler {

        /**
         * Simple handler that shows progress on terminal by counting
         * the number of nodes, ways, and relations already read and
         * printing those counts to stdout.
         *
         * If stdout is not a terminal, nothing is printed.
         *
         * Note that this handler will hide the cursor. If the program
         * is terminated before the final handler is called, it will not
         * be re-activated. Call show_cursor() from an interrupt
         * handler or similar, to do this properly.
         */
        class Progress : public Base {

            uint64_t m_count_nodes;
            uint64_t m_count_ways;
            uint64_t m_count_relations;

            int m_step;

            bool m_is_a_tty;

            timeval m_first_node;
            timeval m_first_way;
            timeval m_first_relation;

            void update_display(bool show_per_second=true) const {
                std::cout << "[" << m_count_nodes << "]";
                if (m_count_ways > 0 || m_count_relations > 0) {
                    std::cout << " [" << m_count_ways << "]";
                    if (m_count_relations > 0) {
                        std::cout << " [" << m_count_relations << "]";
                    }
                }

                if (show_per_second) {
                    timeval now;
                    gettimeofday(&now, 0);

                    if (m_count_relations > 0) {
                        float relation_diff = (now.tv_sec - m_first_relation.tv_sec) * 1000000 + (now.tv_usec - m_first_relation.tv_usec);
                        int relations_per_sec = static_cast<float>(m_count_relations) / relation_diff * 1000000;
                        std::cout << " (" << relations_per_sec << " Relations per second)   ";
                    } else if (m_count_ways > 0) {
                        float way_diff = (now.tv_sec - m_first_way.tv_sec) * 1000000 + (now.tv_usec - m_first_way.tv_usec);
                        int ways_per_sec = static_cast<float>(m_count_ways) / way_diff * 1000000;
                        std::cout << " (" << ways_per_sec << " Ways per second)   ";
                    } else if (m_count_nodes > 0) {
                        float node_diff = (now.tv_sec - m_first_node.tv_sec) * 1000000 + (now.tv_usec - m_first_node.tv_usec);
                        int nodes_per_sec = static_cast<float>(m_count_nodes) / node_diff * 1000000;
                        std::cout << " (" << nodes_per_sec << " Nodes per second)   ";
                    }
                } else {
                    std::cout << "                                   ";
                }

                std::cout << "\r";
                std::cout.flush();
            }

        public:

            /**
             * Initialize handler.
             * @param step after how many nodes/ways/relations the display
             *             should be updated. (default 1000).
             */
            Progress(int step=1000) :
                Base(),
                m_count_nodes(0),
                m_count_ways(0),
                m_count_relations(0),
                m_step(step),
                m_is_a_tty(isatty(1)),
                m_first_node(),
                m_first_way(),
                m_first_relation() {
            }

            void hide_cursor() const {
                std::cout << "\x1b[?25l";
            }

            void show_cursor() const {
                std::cout << "\x1b[?25h";
            }

            void init(const Osmium::OSM::Meta&) const {
                if (m_is_a_tty) {
                    hide_cursor();
                    update_display();
                }
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& /*object*/) {
                if (m_first_node.tv_sec == 0) {
                    gettimeofday(&m_first_node, 0);
                }
                if (m_is_a_tty && ++m_count_nodes % m_step == 0) {
                    update_display();
                }
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& /*object*/) {
                if (m_first_way.tv_sec == 0) {
                    gettimeofday(&m_first_way, 0);
                }
                if (m_is_a_tty && ++m_count_ways % m_step == 0) {
                    update_display();
                }
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& /*object*/) {
                if (m_first_relation.tv_sec == 0) {
                    gettimeofday(&m_first_relation, 0);
                }
                if (m_is_a_tty && ++m_count_relations % m_step == 0) {
                    update_display();
                }
            }

            void final() const {
                if (! m_is_a_tty) {
                    return;
                }

                update_display(false);

                std::cout << "\n  Average: ";

                timeval now;
                gettimeofday(&now, 0);

                if (m_count_nodes > 0) {
                    float node_diff = (m_first_way.tv_sec - m_first_node.tv_sec) * 1000000 + (m_first_way.tv_usec - m_first_node.tv_usec);
                    int nodes_per_sec = static_cast<float>(m_count_nodes) / node_diff * 1000000;
                    std::cout << nodes_per_sec << " Nodes ";
                }

                if (m_count_ways > 0) {
                    float way_diff = (m_first_relation.tv_sec - m_first_way.tv_sec) * 1000000 + (m_first_relation.tv_usec - m_first_way.tv_usec);
                    int ways_per_sec = static_cast<float>(m_count_ways) / way_diff * 1000000;
                    std::cout << ways_per_sec << " Ways ";
                }

                if (m_count_relations > 0) {
                    float relation_diff = (now.tv_sec - m_first_relation.tv_sec) * 1000000 + (now.tv_usec - m_first_relation.tv_usec);
                    int relations_per_sec = static_cast<float>(m_count_relations) / relation_diff * 1000000;
                    std::cout << relations_per_sec << " Relations ";
                }

                show_cursor();
                std::cout  << "per second\n";
                std::cout.flush();
            }

        }; // class Progress

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_PROGRESS_HPP
