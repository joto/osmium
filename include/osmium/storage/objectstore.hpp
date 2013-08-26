#ifndef OSMIUM_STORAGE_OBJECTSTORE_HPP
#define OSMIUM_STORAGE_OBJECTSTORE_HPP

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

#include <algorithm>
#include <set>
#include <boost/bind.hpp>

#include <osmium/handler.hpp>

namespace Osmium {

    namespace Storage {

        /**
         * Stores Nodes, Ways, and Relations in main memory. Can store multiple
         * versions of the same object. Storage is ordered by id and version.
         * Stored objects are const, they can not be changed.
         *
         * The object store uses the handler interface, so storage is simply by
         * calling the node(), way(), and relation() methods like in any other
         * handler.
         *
         * Note that Osmium OSM objects are rather larger, so this store can use
         * a lot of memory. In many cases you don't need the whole object but only
         * parts of it, so using a more space efficient storage might be possible.
         */
        class ObjectStore : public Osmium::Handler::Base {

        public:

            ObjectStore() :
                Base(),
                m_nodes(),
                m_ways(),
                m_relations() {
            }

            /**
             * Insert shared_ptr of Node into object store.
             */
            void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                m_nodes.insert(node);
            }

            /**
             * Insert shared_ptr of Way into object store.
             */
            void way(const shared_ptr<Osmium::OSM::Way const>& way) {
                m_ways.insert(way);
            }

            /**
             * Insert shared_ptr of Relation into object store.
             */
            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                m_relations.insert(relation);
            }

            /**
             * Remove all nodes from object store.
             */
            void clear_nodes() {
                m_nodes.clear();
            }

            /**
             * Remove all ways from object store.
             */
            void clear_ways() {
                m_ways.clear();
            }

            /**
             * Remove all relations from object store.
             */
            void clear_relations() {
                m_relations.clear();
            }

            /**
             * Remove all objects from object store.
             */
            void clear() {
                clear_nodes();
                clear_ways();
                clear_relations();
            }

            /**
             * Feed contents of object store to the given handler. Because
             * objects are stored ordered by id and version, they will be
             * fed to the handler in order.
             *
             * If clear is set, all nodes/ways/relations are removed from the
             * object store after the after_nodes/ways/relations() call to the
             * handler.
             *
             * @tparam THandler Handler class.
             * @param handler Pointer to handler.
             * @param meta Reference to Osmium::OSM::Meta object which will be given to init() method of handler.
             * @param clear Should objects be cleared from the object store? Default is true.
             */
            template <class THandler>
            void feed_to(THandler* handler, Osmium::OSM::Meta& meta, bool clear=true) {
                handler->init(meta);

                handler->before_nodes();
                std::for_each(m_nodes.begin(), m_nodes.end(), boost::bind(&THandler::node, handler, _1));
                handler->after_nodes();
                if (clear) {
                    clear_nodes();
                }

                handler->before_ways();
                std::for_each(m_ways.begin(), m_ways.end(), boost::bind(&THandler::way, handler, _1));
                handler->after_ways();
                if (clear) {
                    clear_ways();
                }

                handler->before_relations();
                std::for_each(m_relations.begin(), m_relations.end(), boost::bind(&THandler::relation, handler, _1));
                handler->after_relations();
                if (clear) {
                    clear_relations();
                }

                handler->final();
            }

        private:

            typedef std::set<shared_ptr<Osmium::OSM::Node const>     > nodeset;
            typedef std::set<shared_ptr<Osmium::OSM::Way const>      > wayset;
            typedef std::set<shared_ptr<Osmium::OSM::Relation const> > relationset;

            nodeset     m_nodes;
            wayset      m_ways;
            relationset m_relations;

        public:

            /**
             * Handler that inserts objects from the store in the right
             * position in the stream of objects it gets and forwards all
             * objects to another handler.
             *
             * Do not change the object store while this handler is active.
             */
            template <class THandler>
            class ApplyHandler : public Osmium::Handler::Forward<THandler> {

            public:

                ApplyHandler(ObjectStore& object_store, THandler& handler, Osmium::OSM::Meta& meta) :
                    Osmium::Handler::Forward<THandler>(handler),
                    m_object_store(object_store),
                    m_handler(handler),
                    m_meta(meta),
                    m_nodes_iter(object_store.m_nodes.begin()),
                    m_nodes_end(object_store.m_nodes.end()),
                    m_ways_iter(object_store.m_ways.begin()),
                    m_ways_end(object_store.m_ways.end()),
                    m_relations_iter(object_store.m_relations.begin()),
                    m_relations_end(object_store.m_relations.end()) {
                }

                void init(const Osmium::OSM::Meta&) {
                    m_handler.init(m_meta);
                }

                void node(const shared_ptr<Osmium::OSM::Node>& node) {
                    while (m_nodes_iter != m_nodes_end && **m_nodes_iter < *node) {
                        m_handler.node(*m_nodes_iter++);
                    }
                    m_handler.node(node);
                }

                void after_nodes() {
                    while (m_nodes_iter != m_nodes_end) {
                        m_handler.node(*m_nodes_iter++);
                    }
                    m_handler.after_nodes();
                    m_object_store.clear_nodes();
                }

                void way(const shared_ptr<Osmium::OSM::Way>& way) {
                    while (m_ways_iter != m_ways_end && **m_ways_iter < *way) {
                        m_handler.way(*m_ways_iter++);
                    }
                    m_handler.way(way);
                }

                void after_ways() {
                    while (m_ways_iter != m_ways_end) {
                        m_handler.way(*m_ways_iter++);
                    }
                    m_handler.after_ways();
                    m_object_store.clear_ways();
                }

                void relation(const shared_ptr<Osmium::OSM::Relation>& relation) {
                    while (m_relations_iter != m_relations_end && **m_relations_iter < *relation) {
                        m_handler.relation(*m_relations_iter++);
                    }
                    m_handler.relation(relation);
                }

                void after_relations() {
                    while (m_relations_iter != m_relations_end) {
                        m_handler.relation(*m_relations_iter++);
                    }
                    m_handler.after_relations();
                    m_object_store.clear_relations();
                }

            private:

                ObjectStore& m_object_store;
                THandler& m_handler;
                Osmium::OSM::Meta& m_meta;

                ObjectStore::nodeset::iterator     m_nodes_iter;
                ObjectStore::nodeset::iterator     m_nodes_end;
                ObjectStore::wayset::iterator      m_ways_iter;
                ObjectStore::wayset::iterator      m_ways_end;
                ObjectStore::relationset::iterator m_relations_iter;
                ObjectStore::relationset::iterator m_relations_end;

            }; // class ApplyHandler

        }; // class ObjectStore

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_OBJECTSTORE_HPP
