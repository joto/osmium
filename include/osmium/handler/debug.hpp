#ifndef OSMIUM_HANDLER_DEBUG_HPP
#define OSMIUM_HANDLER_DEBUG_HPP

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

namespace Osmium {

    namespace Handler {

        class Debug : public Base {

        public:

            Debug() : Base() {
            }

            void callback_node(const OSM::Node *node) const {
                std::cout << "node:\n";
                print_meta(node);
                std::cout << "  lon=" << std::fixed << std::setprecision(7) << node->get_lon() << "\n";
                std::cout << "  lat=" << std::fixed << std::setprecision(7) << node->get_lat() << "\n";
            }

            void callback_way(const OSM::Way *way) const {
                std::cout << "way:\n";
                print_meta(way);
                std::cout << "  node_count=" << way->node_count() << "\n";
                std::cout << "  nodes:\n";
                for (osm_sequence_id_t i=0; i < way->node_count(); ++i) {
                    std::cout << "    ref=" << way->get_node_id(i) << "\n";
                }
            }

            void callback_relation(OSM::Relation *relation) const {
                std::cout << "relation:\n";
                print_meta(relation);
                std::cout << "  member_count=" << relation->member_count() << "\n";
                std::cout << "  members:\n";
                for (osm_sequence_id_t i=0; i < relation->member_count(); ++i) {
                    const Osmium::OSM::RelationMember* m = relation->get_member(i);
                    std::cout << "    type=" << m->type << " ref=" << m->ref << " role=|" << m->role << "|" << "\n";
                }
            }

        private:

            void print_meta(const OSM::Object *object) const {
                std::cout << "  id="   << object->get_id() << "\n";
                std::cout << "  version="   << object->get_version() << "\n";
                std::cout << "  uid="       << object->get_uid() << "\n";
                std::cout << "  user=|"      << object->get_user() << "|\n";
                std::cout << "  changeset=" << object->get_changeset() << "\n";
                std::cout << "  timestamp=" << object->get_timestamp_as_string() << "\n";
                std::cout << "  tags:" << "\n";
                for (int i=0; i < object->tag_count(); ++i) {
                    std::cout << "    k=|" << object->get_tag_key(i) << "| v=|" << object->get_tag_value(i) << "|" << "\n";
                }
            }

        }; // class Debug

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_DEBUG_HPP
