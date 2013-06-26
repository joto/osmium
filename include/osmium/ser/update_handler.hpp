#ifndef OSMIUM_SER_UPDATE_HANDLER_HPP
#define OSMIUM_SER_UPDATE_HANDLER_HPP

/*

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium/osm/meta.hpp>
#include <osmium/ser/item.hpp>

namespace Osmium {

    namespace Ser {

        namespace UpdateHandler {

            class Base : boost::noncopyable {

            public:

                Base() {
                }

                // Destructor is not virtual as this class is not intended to be used polymorphically
                ~Base() {
                }

                void init(Osmium::OSM::Meta&) const {
                }

                void before_nodes() const {
                }

                void node(const Osmium::Ser::Node&, const Osmium::Ser::Node*) const {
                }

                void after_nodes() const {
                }

                void before_ways() const {
                }

                void way(const Osmium::Ser::Way&, const Osmium::Ser::Way*) const {
                }

                void after_ways() const {
                }

                void before_relations() const {
                }

                void relation(const Osmium::Ser::Relation&, const Osmium::Ser::Relation*) const {
                }

                void after_relations() const {
                }

                void final() const {
                }

            }; // class Base

            template <class TMapNodeToWay, class TMapNodeToRelation, class TMapWayToRelation, class TMapRelationToRelation>
            class ObjectsWithDeps : public Base {

            public:

                ObjectsWithDeps(TMapNodeToWay& map_node2way,
                        TMapNodeToRelation& map_node2relation,
                        TMapWayToRelation& map_way2relation,
                        TMapRelationToRelation& map_relation2relation) :
                    Base(),
                    m_map_node2way(map_node2way),
                    m_map_node2relation(map_node2relation),
                    m_map_way2relation(map_way2relation),
                    m_map_relation2relation(map_relation2relation) {
                }

                void node(const Osmium::Ser::Node& new_node, const Osmium::Ser::Node* old_node) const {
                }

                void way(const Osmium::Ser::Way& new_way, const Osmium::Ser::Way* old_way) const {
                    const Osmium::Ser::WayNodeList& wnl = new_way.nodes();
                    for (Osmium::Ser::WayNodeList::iterator it = wnl.begin(); it != wnl.end(); ++it) {
                        m_map_node2way.set(it->ref(), new_way.id());
                    }
                }

                void relation(const Osmium::Ser::Relation& new_relation, const Osmium::Ser::Relation* old_relation) const {
                    const Osmium::Ser::RelationMemberList& rml = new_relation.members();
                    for (Osmium::Ser::RelationMemberList::iterator it = rml.begin(); it != rml.end(); ++it) {
                        switch (it->type().t()) {
                            case Osmium::Ser::ItemType::itemtype_node:
                                m_map_node2relation.set(it->ref(), new_relation.id());
                                break;
                            case Osmium::Ser::ItemType::itemtype_way:
                                m_map_way2relation.set(it->ref(), new_relation.id());
                                break;
                            case Osmium::Ser::ItemType::itemtype_relation:
                                m_map_relation2relation.set(it->ref(), new_relation.id());
                                break;
                        }
                    }
                }

            private:

                TMapNodeToWay&          m_map_node2way;
                TMapNodeToRelation&     m_map_node2relation;
                TMapWayToRelation&      m_map_way2relation;
                TMapRelationToRelation& m_map_relation2relation;

            }; // class ObjectsWithDeps

        } // namespace UpdateHandler

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_UPDATE_HANDLER_HPP
