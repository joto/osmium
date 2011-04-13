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
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
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

            void callback_object(OSM::Object *object) const {
                std::cout << "object:" << std::endl;
                std::cout << "  id="   << object->get_id() << std::endl;
                std::cout << "  version="   << object->get_version() << std::endl;
                std::cout << "  uid="       << object->get_uid() << std::endl;
                std::cout << "  user=|"      << object->get_user() << "|" << std::endl;
                std::cout << "  changeset=" << object->get_changeset() << std::endl;
                std::cout << "  timestamp=" << object->get_timestamp_str() << std::endl;
                std::cout << "  tags:" << std::endl;
                for (int i=0; i < object->tag_count(); i++) {
                    std::cout << "    k=|" << object->get_tag_key(i) << "| v=|" << object->get_tag_value(i) << "|" << std::endl;
                }
            }

            void callback_node(const OSM::Node *object) const {
                std::cout << "  node:" << std::endl;
                std::cout << "    lon=" << object->get_lon() << std::endl;
                std::cout << "    lat=" << object->get_lat() << std::endl;
            }

            void callback_way(const OSM::Way *object) const {
                std::cout << "  way:" << std::endl;
                std::cout << "    node_count=" << object->node_count() << std::endl;
                std::cout << "    nodes:" << std::endl;
                for (int i=0; i < object->node_count(); i++) {
                    std::cout << "      ref=" << object->nodes[i] << std::endl;
                }
            }

            void callback_relation(OSM::Relation *object) const {
                std::cout << "  relation:" << std::endl;
                std::cout << "    member_count=" << object->member_count() << std::endl;
                std::cout << "    members:" << std::endl;
                for (int i=0; i < object->member_count(); i++) {
                    const Osmium::OSM::RelationMember *m = object->get_member(i);
                    std::cout << "      type=" << m->type << " ref=" << m->ref << " role=|" << m->role << "|" << std::endl;
                }
            }

        }; // class Debug

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_DEBUG_HPP
