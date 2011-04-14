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

        template <class THandler>
        class Base {

        public:

            OSM::Node     *node;
            OSM::Way      *way;
            OSM::Relation *relation;

            THandler *handler;
            bool delete_handler_on_destruction;

            Base(THandler *h) __attribute__((noinline)) : handler(h) {
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

            virtual ~Base() __attribute__((noinline)) {
                handler->callback_final();

                if (delete_handler_on_destruction) {
                    delete handler;
                }

                delete relation;
                delete way;
                delete node;
            }

            virtual void parse() = 0;

        };

    } // namespace Input

} // namespace Osmium

#include <osmium/input/xml.hpp>
#include <osmium/input/pbf.hpp>

#endif // OSMIUM_INPUT_HPP
