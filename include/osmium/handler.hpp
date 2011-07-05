#ifndef OSMIUM_HANDLER_HPP
#define OSMIUM_HANDLER_HPP

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

#include <osmium/osm/meta.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/area.hpp>

namespace Osmium {

    /**
     * @brief Handlers operate on %OSM data through callbacks.
     *
     * All handlers should use Osmium::Handler::Base as a public
     * base class. See its documentation for details.
     */
    namespace Handler {

        /**
         * Base class for all handler classes.
         * Defines empty methods that can be overwritten in child classes.
         *
         * To define your own handler create a subclass of this class.
         * Only overwrite the functions you actually use. They must be declared public.
         * If you overwrite the constructor call the Base constructor without arguments.
         */
        class Base {

        public:

            Base() {
            }

            void init(Osmium::OSM::Meta&) const {
            }

            void before_nodes() const {
            }

            void node(Osmium::OSM::Node*) const {
            }

            void after_nodes() const {
            }

            void before_ways() const {
            }

            void way(Osmium::OSM::Way*) const {
            }

            void after_ways() const {
            }

            void before_relations() const {
            }

            void relation(Osmium::OSM::Relation*) const {
            }

            void after_relations() const {
            }

            void area(Osmium::OSM::Area*) const {
            }

            void final() const {
            }

        }; // class Base

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_HPP
