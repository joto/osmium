#ifndef OSMIUM_HANDLER_HPP
#define OSMIUM_HANDLER_HPP

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

#include <boost/utility.hpp>

#include <osmium/debug.hpp>

#include <osmium/osm/meta.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>

namespace Osmium {

    namespace OSM {
       class Area;
    }  // namespace OSM

    /**
     * @brief Handlers operate on %OSM data through callbacks.
     *
     * All handlers should use Osmium::Handler::Base as a public
     * base class. See its documentation for details.
     */
    namespace Handler {

        /**
         * Handlers can throw this exception to show that they are done.
         * When a handler, for instance, is only interested in nodes, it
         * can throw this in the after_nodes() method. The parser will
         * stop reading the input file after this.
         *
         * Note that when you write a handler that calls other handlers
         * that can throw this, you might have to catch this exception
         * in your handler.
         */
        class StopReading {
        };

        /**
         * Base class for all handler classes.
         * Defines empty methods that can be overwritten in child classes.
         *
         * To define your own handler create a subclass of this class.
         * Only overwrite the methods you actually use. They must be declared public.
         * If you overwrite the constructor, call the Base constructor without arguments.
         */
        class Base : boost::noncopyable, public Osmium::WithDebug {

        public:

            Base() :
                Osmium::WithDebug() {
            }

            // Destructor is not virtual as this class is not intended to be used polymorphically
            ~Base() {
            }

            void init(Osmium::OSM::Meta&) const {
            }

            void before_nodes() const {
            }

            void node(const shared_ptr<Osmium::OSM::Node const>&) const {
            }

            void after_nodes() const {
            }

            void before_ways() const {
            }

            void way(const shared_ptr<Osmium::OSM::Way const>&) const {
            }

            void after_ways() const {
            }

            void before_relations() const {
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>&) const {
            }

            void after_relations() const {
            }

            void area(const shared_ptr<Osmium::OSM::Area const>&) const {
            }

            void final() const {
            }

        }; // class Base

        /**
         * This handler forwards all calls to another handler.
         * Use this as a base for your handler instead of Base() if you want calls
         * forwarded by default.
         */
        template <class THandler>
        class Forward : public Base {

        public:

            Forward(THandler& next_handler) :
                Base(),
                m_next_handler(next_handler) {
            }

            void init(Osmium::OSM::Meta& meta) const {
                m_next_handler.init(meta);
            }

            void before_nodes() const {
                m_next_handler.before_nodes();
            }

            void node(const shared_ptr<Osmium::OSM::Node>& node) const {
                m_next_handler.node(node);
            }

            void after_nodes() const {
                m_next_handler.after_nodes();
            }

            void before_ways() const {
                m_next_handler.before_ways();
            }

            void way(const shared_ptr<Osmium::OSM::Way>& way) const {
                m_next_handler.way(way);
            }

            void after_ways() const {
                m_next_handler.after_ways();
            }

            void before_relations() const {
                m_next_handler.before_relations();
            }

            void relation(const shared_ptr<Osmium::OSM::Relation>& relation) const {
                m_next_handler.relation(relation);
            }

            void after_relations() const {
                m_next_handler.after_relations();
            }

            void area(const shared_ptr<Osmium::OSM::Area>& area) const {
                m_next_handler.area(area);
            }

            void final() const {
                m_next_handler.final();
            }

            void set_debug_level(int debug) {
                m_next_handler.set_debug_level(debug);
            }

        protected:

            THandler& next_handler() const {
                return m_next_handler;
            }

        private:

            THandler& m_next_handler;

        }; // class Forward

        /**
         * This handler calls the two handlers given as argument in sequence
         * in each method.
         */
        template <class THandler1, class THandler2>
        class Sequence {

        public:

            Sequence(THandler1& handler1, THandler2& handler2) :
                m_handler1(handler1),
                m_handler2(handler2) {
            }

            void init(Osmium::OSM::Meta& meta) const {
                m_handler1.init(meta);
                m_handler2.init(meta);
            }

            void before_nodes() const {
                m_handler1.before_nodes();
                m_handler2.before_nodes();
            }

            void node(const shared_ptr<Osmium::OSM::Node>& node) const {
                m_handler1.node(node);
                m_handler2.node(node);
            }

            void after_nodes() const {
                m_handler1.after_nodes();
                m_handler2.after_nodes();
            }

            void before_ways() const {
                m_handler1.before_ways();
                m_handler2.before_ways();
            }

            void way(const shared_ptr<Osmium::OSM::Way>& way) const {
                m_handler1.way(way);
                m_handler2.way(way);
            }

            void after_ways() const {
                m_handler1.after_ways();
                m_handler2.after_ways();
            }

            void before_relations() const {
                m_handler1.before_relations();
                m_handler2.before_relations();
            }

            void relation(const shared_ptr<Osmium::OSM::Relation>& relation) const {
                m_handler1.relation(relation);
                m_handler2.relation(relation);
            }

            void after_relations() const {
                m_handler1.after_relations();
                m_handler2.after_relations();
            }

            void area(const shared_ptr<Osmium::OSM::Area>& area) const {
                m_handler1.area(area);
                m_handler2.area(area);
            }

            void final() const {
                m_handler1.final();
                m_handler2.final();
            }

            void set_debug_level(int debug) {
                m_handler1.set_debug_level(debug);
                m_handler2.set_debug_level(debug);
            }

        private:

            THandler1& m_handler1;
            THandler2& m_handler2;

        }; // class Sequence

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_HPP
