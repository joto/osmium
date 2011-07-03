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
     * @brief Input classes parse OSM files and call a handler on the data they read.
     */
    namespace Input {

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
         * Virtual base class for all input classes.
         *
         * The THandler template parameter of this class (and child classes)
         * names a policy class on which methods will be called. The class
         * should implement one or more of the following functions:
         *
         * - init(Osmium::OSM::Meta&)
         * - before_nodes/ways/relations()
         * - node/way/relation(Osmium::OSM::Node/Way/Relation*)
         * - after_nodes/ways/relations()
         * - final()
         * - area(Osmium::OSM::Area*)
         *
         * init() will be called before all others, final()
         * after all others.
         *
         * For every object node(), way(), or
         * relation() will be called, respectively.
         *
         * When there are several objects of the same type in a row the
         * before_*() function will be called before them and the
         * after_*() function after them. If your input file is
         * sorted as OSM files normally are, i.e. all nodes, then all ways,
         * then all relations, this will call before_nodes() once,
         * then for all the nodes node(), then after_nodes(),
         * then before_ways(), and so on.
         * This will also work properly if the input file contains, say, first
         * all relations, than all ways and then all nodes.
         *
         * But if you have nodes, ways, and relations intermixed in an input
         * file these handlers will probably not called in a useful way for you.
         * You can use osmosis --sort to sort your input file first.
         *
         * The method area() is special. It will only be called if
         * you have the multipolygon handler before your handler. There are no
         * before/after_areas() methods. Use init() and
         * final() instead.
         */
        template <class THandler>
        class Base {

        public:

            virtual ~Base() {
                m_handler.final();

                delete m_relation;
                delete m_way;
                delete m_node;
            }

            /**
             * Parse an OSM input file. This is a pure virtual function,
             * it must be overwritten in a child class of Osmium::Input::Base.
             */
            virtual void parse() = 0;

        protected:

            Base(Osmium::OSMFile& file,
                 THandler& handler)
                : m_last_object_type(UNKNOWN),
                  m_file(file),
                  m_handler(handler),
                  m_meta() {

                m_file.open_for_input();

                m_node     = new Osmium::OSM::Node;
                m_way      = new Osmium::OSM::Way(2000); // create way object with space for 2000 nodes
                m_relation = new Osmium::OSM::Relation;
            }

            void call_after_and_before_handlers(osm_object_type_t current_object_type) {
                if (current_object_type != m_last_object_type) {
                    switch (m_last_object_type) {
                        case NODE:
                            m_handler.after_nodes();
                            break;
                        case WAY:
                            m_handler.after_ways();
                            break;
                        case RELATION:
                            m_handler.after_relations();
                            break;
                        default:
                            break;
                    }
                    switch (current_object_type) {
                        case NODE:
                            if (m_last_object_type == UNKNOWN) {
                                m_handler.init(m_meta);
                            }
                            m_handler.before_nodes();
                            break;
                        case WAY:
                            m_handler.before_ways();
                            break;
                        case RELATION:
                            m_handler.before_relations();
                            break;
                        default:
                            break;
                    }
                    m_last_object_type = current_object_type;
                }
            }

            Osmium::OSM::Meta& meta() {
                return m_meta;
            }

            int get_fd() const {
                return m_file.get_fd();
            }

            const Osmium::OSMFile& get_file() const {
                return m_file;
            }

            void handle_node() {
                m_handler.node(m_node);
            }

            void handle_way() {
                m_handler.way(m_way);
            }

            void handle_relation() {
                m_handler.relation(m_relation);
            }

            Osmium::OSM::Node* node() {
                return m_node;
            }

            Osmium::OSM::Way* way() {
                return m_way;
            }

            Osmium::OSM::Relation* relation() {
                return m_relation;
            }

        private:

            /**
             * The last object type we read (before the current one).
             * Used to properly call before and after methods.
             */
            osm_object_type_t m_last_object_type;

            /**
             * The OSMFile we opened this file with.
             */
            Osmium::OSMFile m_file;

            /**
             * Handler we will call callbacks on.
             */
            THandler& m_handler;

            Osmium::OSM::Meta m_meta;

            Osmium::OSM::Node*     m_node;
            Osmium::OSM::Way*      m_way;
            Osmium::OSM::Relation* m_relation;

        };

    } // namespace Input

} // namespace Osmium

#include <osmium/input/xml.hpp>
#include <osmium/input/pbf.hpp>

#endif // OSMIUM_INPUT_HPP
