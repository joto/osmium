#ifndef OSMIUM_INPUT_HPP
#define OSMIUM_INPUT_HPP

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

#include <osmium/smart_ptr.hpp>
#include <osmium/osmfile.hpp>
#include <osmium/handler.hpp>

namespace Osmium {

    /**
     * @brief %Input classes parse %OSM files and call a handler on the data they read.
     */
    namespace Input {

        /**
         * Virtual base class for all input classes.
         *
         * The THandler template parameter of this class (and child classes)
         * names a policy class on which methods will be called. The class
         * should implement one or more of the following functions:
         *
         * - init(Osmium::OSM::Meta&)
         * - before_nodes/ways/relations()
         * - node/way/relation(const shared_ptr<Osmium::OSM::Node/Way/Relation>&)
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
        class Base : boost::noncopyable {

        public:

            virtual ~Base() {
            }

            /**
             * Parse an OSM input file. This is a pure virtual function,
             * it must be overwritten in a child class of Osmium::Input::Base.
             */
            virtual void parse() = 0;

        protected:

            Base(const Osmium::OSMFile& file,
                 THandler& handler) :
                m_last_object_type(UNKNOWN),
                m_file(file),
                m_handler(handler),
                m_meta(),
                m_node(),
                m_way(),
                m_relation() {

                m_meta.has_multiple_object_versions(m_file.has_multiple_object_versions());
                m_file.open_for_input();

            }

            void call_after_and_before_on_handler(osm_object_type_t current_object_type) {
                if (current_object_type != m_last_object_type) {
                    switch (m_last_object_type) {
                        case UNKNOWN:
                            m_handler.init(m_meta);
                            break;
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

            void call_node_on_handler() const {
                m_handler.node(m_node);
            }

            void call_way_on_handler() const {
                m_handler.way(m_way);
            }

            void call_relation_on_handler() const {
                m_handler.relation(m_relation);
            }

            void call_final_on_handler() const {
                m_handler.final();
            }

            Osmium::OSM::Meta& meta() {
                return m_meta;
            }

            int fd() const {
                return m_file.fd();
            }

            const Osmium::OSMFile& file() const {
                return m_file;
            }

            /*
               The following methods prepare the m_node/way/relation member
               variable for use. If it is empty or in use by somebody other
               than this parser, a new object will be allocated. If it not is
               use, it will be reset to it's pristine state by calling the
               destructor directly and then placement new. This gets around a
               memory deallocation and re-allocation which was timed to slow
               down the program noticably.
            */
            Osmium::OSM::Node& prepare_node() {
                if (m_node && m_node.unique()) {
                    m_node->~Node();
                    new (m_node.get()) Osmium::OSM::Node();
                } else {
                    m_node = make_shared<Osmium::OSM::Node>();
                }
                return *m_node;
            }

            Osmium::OSM::Way& prepare_way() {
                if (m_way && m_way.unique()) {
                    m_way->~Way();
                    new (m_way.get()) Osmium::OSM::Way(2000);
                } else {
                    m_way = make_shared<Osmium::OSM::Way>(2000);
                }
                return *m_way;
            }

            Osmium::OSM::Relation& prepare_relation() {
                if (m_relation && m_relation.unique()) {
                    m_relation->~Relation();
                    new (m_relation.get()) Osmium::OSM::Relation();
                } else {
                    m_relation = make_shared<Osmium::OSM::Relation>();
                }
                return *m_relation;
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

        protected:

            shared_ptr<Osmium::OSM::Node>     m_node;
            shared_ptr<Osmium::OSM::Way>      m_way;
            shared_ptr<Osmium::OSM::Relation> m_relation;

        }; // class Base

    } // namespace Input

} // namespace Osmium

#endif // OSMIUM_INPUT_HPP
