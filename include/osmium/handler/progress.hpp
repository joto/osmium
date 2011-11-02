#ifndef OSMIUM_HANDLER_PROGRESS_HPP
#define OSMIUM_HANDLER_PROGRESS_HPP

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

#include <unistd.h>
#include <sys/time.h>

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

            uint64_t count_nodes;
            uint64_t count_ways;
            uint64_t count_relations;

            int step;

            bool is_a_tty;

            timeval first_node;
            timeval first_way;
            timeval first_relation;

            void update_display(bool show_per_second=true) const {
                std::cout << "[" << count_nodes << "]";
                if (count_ways > 0 || count_relations > 0) {
                    std::cout << " [" << count_ways << "]";
                    if (count_relations > 0) {
                        std::cout << " [" << count_relations << "]";
                    }
                }

                if (show_per_second) {
                    timeval now;
                    gettimeofday(&now, 0);

                    if (count_relations > 0) {
                        float relation_diff = (now.tv_sec - first_relation.tv_sec) * 1000000 + (now.tv_usec - first_relation.tv_usec);
                        int relations_per_sec = (float)count_relations / relation_diff * 1000000;
                        std::cout << " (" << relations_per_sec << " Relations per second)   ";
                    } else if (count_ways > 0) {
                        float way_diff = (now.tv_sec - first_way.tv_sec) * 1000000 + (now.tv_usec - first_way.tv_usec);
                        int ways_per_sec = (float)count_ways / way_diff * 1000000;
                        std::cout << " (" << ways_per_sec << " Ways per second)   ";
                    } else if (count_nodes > 0) {
                        float node_diff = (now.tv_sec - first_node.tv_sec) * 1000000 + (now.tv_usec - first_node.tv_usec);
                        int nodes_per_sec = (float)count_nodes / node_diff * 1000000;
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
             * @param s Step, after how many nodes/ways/relations the display
             *          should be updated. (default 1000).
             */
            Progress(int s=1000) : Base(), count_nodes(0), count_ways(0), count_relations(0), step(s), is_a_tty(false), first_node(), first_way(), first_relation() {
                if (isatty(1)) {
                    is_a_tty = true;
                }
            }

            void hide_cursor() const {
                std::cout << "\x1b[?25l";
            }

            void show_cursor() const {
                std::cout << "\x1b[?25h";
            }

            void init(Osmium::OSM::Meta&) const {
                if (is_a_tty) {
                    hide_cursor();
                    update_display();
                }
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& /*object*/) {
                if (first_node.tv_sec == 0) {
                    gettimeofday(&first_node, 0);
                }
                if (is_a_tty && ++count_nodes % step == 0) {
                    update_display();
                }
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& /*object*/) {
                if (first_way.tv_sec == 0) {
                    gettimeofday(&first_way, 0);
                }
                if (is_a_tty && ++count_ways % step == 0) {
                    update_display();
                }
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& /*object*/) {
                if (first_relation.tv_sec == 0) {
                    gettimeofday(&first_relation, 0);
                }
                if (is_a_tty && ++count_relations % step == 0) {
                    update_display();
                }
            }

            void final() const {
                if (is_a_tty) {
                    update_display(false);
                    std::cout << std::endl;

                    std::cout << "  Average: ";

                    timeval now;
                    gettimeofday(&now, 0);

                    if (count_nodes > 0) {
                        float node_diff = (first_way.tv_sec - first_node.tv_sec) * 1000000 + (first_way.tv_usec - first_node.tv_usec);
                        int nodes_per_sec = (float)count_nodes / node_diff * 1000000;
                        std::cout << nodes_per_sec << " Nodes ";
                    }

                    if (count_ways > 0) {
                        float way_diff = (first_relation.tv_sec - first_way.tv_sec) * 1000000 + (first_relation.tv_usec - first_way.tv_usec);
                        int ways_per_sec = (float)count_ways / way_diff * 1000000;
                        std::cout << ways_per_sec << " Ways ";
                    }

                    if (count_relations > 0) {
                        float relation_diff = (now.tv_sec - first_relation.tv_sec) * 1000000 + (now.tv_usec - first_relation.tv_usec);
                        int relations_per_sec = (float)count_relations / relation_diff * 1000000;
                        std::cout << relations_per_sec << " Relations ";
                    }

                    show_cursor();
                    std::cout  << "per second" << std::endl;
                    std::cout.flush();
                }
            }

        }; // class Progress

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_PROGRESS_HPP
