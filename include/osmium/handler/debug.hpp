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

            void init(Osmium::OSM::Meta& meta) const {
                std::cout << "meta:\n";
                if (meta.bounds().defined()) {
                    std::cout << "  bounds=" << meta.bounds() << "\n";
                }
            }

            void node(const Osmium::OSM::Node* node) const {
                std::cout << "node:\n";
                print_meta(node);
                const Osmium::OSM::Position position = node->position();
                std::cout << "  lon=" << std::fixed << std::setprecision(7) << position.lon() << "\n";
                std::cout << "  lat=" << std::fixed << std::setprecision(7) << position.lat() << "\n";
            }

            void way(const Osmium::OSM::Way* way) const {
                std::cout << "way:\n";
                print_meta(way);
                std::cout << "  node_count=" << way->node_count() << "\n";
                std::cout << "  nodes:\n";
                Osmium::OSM::WayNodeList::const_iterator end = way->nodes().end();
                for (Osmium::OSM::WayNodeList::const_iterator it = way->nodes().begin(); it != end; ++it) {
                    std::cout << "    ref=" << it->ref() << "\n";
                }
            }

            void relation(Osmium::OSM::Relation* relation) const {
                std::cout << "relation:\n";
                print_meta(relation);
                std::cout << "  members.size=" << relation->members().size() << "\n";
                std::cout << "  members:\n";
                Osmium::OSM::RelationMemberList::const_iterator end = relation->members().end();
                for (Osmium::OSM::RelationMemberList::const_iterator it = relation->members().begin(); it != end; ++it) {
                    std::cout << "    type=" << it->type() << " ref=" << it->ref() << " role=|" << it->role() << "|" << "\n";
                }
            }

        private:

            void print_meta(const Osmium::OSM::Object* object) const {
                std::cout << "  id="   << object->get_id() << "\n";
                std::cout << "  version="   << object->get_version() << "\n";
                std::cout << "  uid="       << object->get_uid() << "\n";
                std::cout << "  user=|"      << object->get_user() << "|\n";
                std::cout << "  changeset=" << object->get_changeset() << "\n";
                std::cout << "  timestamp=" << object->get_timestamp_as_string() << "\n";
                std::cout << "  tags:" << "\n";
                Osmium::OSM::TagList::const_iterator end = object->tags().end();
                for (Osmium::OSM::TagList::const_iterator it = object->tags().begin(); it != end; ++it) {
                    std::cout << "    k=|" << it->key() << "| v=|" << it->value() << "|" << "\n";
                }
            }

        }; // class Debug

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_DEBUG_HPP
