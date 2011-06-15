#ifndef OSMIUM_INPUT_HPP
#define OSMIUM_INPUT_HPP

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

#include <osmium/handler.hpp>

namespace Osmium {

    /**
     * @brief Namespace for input classes implementing file parsers.
     */
    namespace Input {

        /**
         * Handlers can throw this exception to show that they are done.
         * When a handler, for instance, is only interested in nodes, it
         * can throw this in the after_nodes() callback. The parser will
         * stop reading the input file after this.
         *
         * Note that when you write a handler that calls other handlers
         * that can throw this, you might have to catch this exception
         * in your handler.
         */
        class StopReading {
        };

        /**
         * Virtual base class for all input classes.
         *
         * The THandler template parameter of this class (and child classes)
         * names a class on which callbacks will be called. The class should
         * implement one or more of the following functions:
         *
         * - callback_init()
         * - callback_before_nodes/ways/relations()
         * - callback_node/way/relation(Osmium::OSM::Node/Way/Relation *)
         * - callback_after_nodes/ways/relations()
         * - callback_final()
         * - callback_multipolygon(Osmium::OSM::Multipolygon *)
         *
         * callback_init() will be called before all others, callback_final()
         * after all others.
         *
         * For every object callback_node(), callback_way(), or
         * callback_relation() will be called, respectively.
         *
         * When there are several objects of the same type in a row the
         * callback_before_*() function will be called before them and the
         * callback_after_*() function after them. If your input file is
         * sorted as OSM files normally are, i.e. all nodes, then all ways,
         * then all relations, this will call callback_before_nodes() once,
         * then for all the nodes callback_node(), then callback_after_nodes(),
         * then callback_before_ways(), and so on.
         * This will also work properly if the input file contains, say, first
         * all relations, than all ways and then all nodes.
         *
         * But if you have nodes, ways, and relations intermixed in an input
         * file these handlers will probably not called in a useful way for you.
         * You can use osmosis --sort to sort your input file first.
         *
         * The callback_multipolygon() is special. It will only be called if
         * you have the multipolygon handler before your handler. There are no
         * before/after_multipolygons() callbacks. Use callback_init() and
         * callback_final() instead.
         */
        template <class THandler>
        class Base {

            /**
             * The last object type we read (before the current one).
             * Used to properly call before and after callbacks.
             */
            osm_object_type_t m_last_object_type;

            /**
             * The OSMFile we opened this file with.
             */
            OSMFile m_file;

            /**
             * Handler we will call callbacks on.
             */
            THandler& m_handler;

        protected:

            OSM::Node     *node;
            OSM::Way      *way;
            OSM::Relation *relation;

            Base(OSMFile& file,
                 THandler& handler)
               : m_last_object_type(UNKNOWN),
                 m_file(file),
                 m_handler(handler)  {

                m_file.open_for_input();

                node     = new Osmium::OSM::Node;
                way      = new Osmium::OSM::Way(2000); // create way object with space for 2000 nodes
                relation = new Osmium::OSM::Relation;

                m_handler.callback_init();
            }

            void call_after_and_before_handlers(osm_object_type_t current_object_type) {
                if (current_object_type != m_last_object_type) {
                    switch (m_last_object_type) {
                        case NODE:
                            m_handler.callback_after_nodes();
                            break;
                        case WAY:
                            m_handler.callback_after_ways();
                            break;
                        case RELATION:
                            m_handler.callback_after_relations();
                            break;
                        default:
                            break;
                    }
                    switch (current_object_type) {
                        case NODE:
                            m_handler.callback_before_nodes();
                            break;
                        case WAY:
                            m_handler.callback_before_ways();
                            break;
                        case RELATION:
                            m_handler.callback_before_relations();
                            break;
                        default:
                            break;
                    }
                    m_last_object_type = current_object_type;
                }
            }

            int get_fd() const {
                return m_file.get_fd();
            }

            const OSMFile& get_file() const {
                return m_file;
            }

            void callback_node() {
                m_handler.callback_node(node);
            }

            void callback_way() {
                m_handler.callback_way(way);
            }

            void callback_relation() {
                m_handler.callback_relation(relation);
            }

        public:

            virtual ~Base() {
                m_handler.callback_final();

                delete relation;
                delete way;
                delete node;
            }

            /**
             * Parse an OSM input file. This is a pure virtual function,
             * it must be overwritten in a child class of Osmium::Input::Base.
             */
            virtual void parse() = 0;

        };

    } // namespace Input

} // namespace Osmium

#include <osmium/input/xml.hpp>
#include <osmium/input/pbf.hpp>

#endif // OSMIUM_INPUT_HPP
