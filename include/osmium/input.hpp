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
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
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
         * - callback_object(Osmium::OSM::Object *)
         * - callback_before_nodes/ways/relations()
         * - callback_node/way/relation(Osmium::OSM::Node/Way/Relation *)
         * - callback_after_nodes/ways/relations()
         * - callback_final()
         * - callback_multipolygon(Osmium::OSM::Multipolygon *)
         *
         * callback_init() will be called before all others, callback_final()
         * after all others.
         *
         * For every object in the input callback_object() will be called
         * followed by callback_node(), callback_way(), or callback_relation(),
         * respectively.
         *
         * When there are several objects of the same type in a row the
         * callback_before_*() function will be called before them and the
         * callback_after_*() function after them. If your input file is
         * sorted as OSM files normally are, i.e. all nodes, then all ways,
         * then all relations, this will call callback_before_nodes() once,
         * then for all the nodes callback_object() and callback_node(), then
         * callback_after_nodes(), then callback_before_ways(), and so on.
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
             * If there was no handler given when constructing this class,
             * we create our own instance of the handler and store here
             * that we need to delete it on destruction.
             */
            bool delete_handler_on_destruction;

        protected:

            OSM::Node     *node;
            OSM::Way      *way;
            OSM::Relation *relation;

            /**
             * Handler we will call callbacks on.
             */
            THandler *handler;

            /**
             * The last object type we read (before the current one).
             * Used to properly call before and after callbacks.
             */
            osm_object_type_t last_object_type;

            Base(THandler *h) __attribute__((noinline)) : handler(h), last_object_type(UNKNOWN) {
                node     = new Osmium::OSM::Node;
                way      = new Osmium::OSM::Way;
                relation = new Osmium::OSM::Relation;

                if (handler) {
                    delete_handler_on_destruction = false;
                } else {
                    handler = new THandler;
                    delete_handler_on_destruction = true;
                }

                handler->callback_init();
            }

            void call_after_and_before_handlers(osm_object_type_t current_object_type) {
                if (current_object_type != last_object_type) {
                    switch (last_object_type) {
                        case NODE:
                            handler->callback_after_nodes();
                            break;
                        case WAY:
                            handler->callback_after_ways();
                            break;
                        case RELATION:
                            handler->callback_after_relations();
                            break;
                        default:
                            break;
                    }
                    switch (current_object_type) {
                        case NODE:
                            handler->callback_before_nodes();
                            break;
                        case WAY:
                            handler->callback_before_ways();
                            break;
                        case RELATION:
                            handler->callback_before_relations();
                            break;
                        default:
                            break;
                    }
                    last_object_type = current_object_type;
                }
            }

        public:

            virtual ~Base() __attribute__((noinline)) {
                handler->callback_final();

                if (delete_handler_on_destruction) {
                    delete handler;
                }

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
