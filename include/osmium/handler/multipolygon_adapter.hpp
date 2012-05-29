#ifndef OSMIUM_HANDLER_MPADAPTER_HPP
#define OSMIUM_HANDLER_MPADAPTER_HPP

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

#include <boost/function.hpp>
#include <osmium/handler.hpp>

#include <osmium/storage/byid/sparsetable.hpp>
#include <osmium/storage/byid/mmap_file.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/handler/multipolygon.hpp>
#include <osmium/geometry/multipolygon.hpp>

namespace Osmium {

    namespace Handler {

        template <class THandler>
        class MultipolygonAdapter : boost::noncopyable {

        public:
            
            /// Inner Class for 1st Pass Processing
            class FirstPass : public Osmium::Handler::Forward<THandler> {
                
            public:
                
                FirstPass(MultipolygonAdapter* parent) : Osmium::Handler::Forward<THandler>(parent->m_handler), m_parent(parent) {}
                
                void before_relations() {
                    m_parent->handler_multipolygon->before_relations();
                }

                void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                    m_parent->handler_multipolygon->relation(relation);
                }

                void after_relations() {
                    m_parent->handler_multipolygon->after_relations();
                }
                
            private:
                
                MultipolygonAdapter* m_parent;
                
            };
            
            
            /// Inner Class for 2nd Pass Processing
            class SecondPass : public Base {
                
            public:
                
                SecondPass(MultipolygonAdapter* parent)  : m_parent(parent) {
                    handler_cfw = new cfw_handler_t(store_pos, store_neg);
                }
                    
                ~SecondPass() {
                    delete handler_cfw;
                }
                
                void init(Osmium::OSM::Meta& meta) {
                    handler_cfw->init(meta);
                }

                void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                    handler_cfw->node(node);
                }

                void after_nodes() {
                    handler_cfw->after_nodes();
                }

                void way(const shared_ptr<Osmium::OSM::Way>& way) {
                    handler_cfw->way(way);
                    m_parent->handler_multipolygon->way(way);
                }
            
            private:

                MultipolygonAdapter* m_parent;
                
                typedef Osmium::Storage::ById::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
                typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;
                typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;
    
                storage_sparsetable_t store_pos;
                storage_mmap_t store_neg;
                cfw_handler_t* handler_cfw;

            };

            
            
            /// Adapter Class
            MultipolygonAdapter(THandler* handler, bool attempt_repair) : m_handler(handler) {
                boost::function<void (Osmium::OSM::Area*)> cbmp_f = std::bind1st(std::mem_fun(&MultipolygonAdapter::cbmp), this);
                handler_multipolygon = new Osmium::Handler::Multipolygon(attempt_repair, cbmp_f);
                p1 = new FirstPass(this);
                p2 = new SecondPass(this);
            }

            ~MultipolygonAdapter() {
                delete p1;
                delete p2;
            }
            
            // static function pointer doesn't have a link to the instance
            void cbmp(Osmium::OSM::Area* area) {
                m_handler->area(area);
            }

            FirstPass *firstPass() const {
                return p1;
            }

            SecondPass *secondPass() const {
                return p2;
            }

        private:

            Osmium::Handler::Multipolygon* handler_multipolygon;

            THandler* m_handler;
            FirstPass *p1;
            SecondPass *p2;

        }; // class MultipolygonAdapter

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_MPADAPTER_HPP
