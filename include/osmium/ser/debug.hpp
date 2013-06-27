#ifndef OSMIUM_SER_DEBUG_HPP
#define OSMIUM_SER_DEBUG_HPP

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

#include <iomanip>
#include <iostream>
#include <string>

#include <osmium/ser/item.hpp>
#include <osmium/utils/timestamp.hpp>

namespace Osmium {

    namespace Ser {

        class Dump {

        public:

            Dump(std::ostream& out, bool with_size=true, const std::string& prefix="") :
                m_out(out),
                m_with_size(with_size),
                m_prefix(prefix) {
            }

            void print_title(const char *title, const Osmium::Ser::TypedItem& item) const {
                m_out << m_prefix << title << ":";
                if (m_with_size) {
                    m_out << " [" << item.size() << "]";
                }
                m_out << "\n";
            }

            void print_meta(const Osmium::Ser::Object& object) const {
                m_out << m_prefix << "  id="        << object.id() << "\n";
                m_out << m_prefix << "  version="   << object.version() << "\n";
                m_out << m_prefix << "  uid="       << object.uid() << "\n";
                m_out << m_prefix << "  user=|"     << object.user() << "|\n";
                m_out << m_prefix << "  changeset=" << object.changeset() << "\n";
                m_out << m_prefix << "  timestamp=" << Osmium::Timestamp::to_iso(object.timestamp()) << "\n";
                Dump dump(m_out, m_with_size, m_prefix + "  ");
                std::for_each(object.begin(), object.end(), dump);
            }

            void print_position(const Osmium::Ser::Node& node) const {
                const Osmium::OSM::Position& position = node.position();
                m_out << m_prefix << "  lon=" << std::fixed << std::setprecision(7) << position.lon() << "\n";
                m_out << m_prefix << "  lat=" << std::fixed << std::setprecision(7) << position.lat() << "\n";
            }

            void print_tag_list(const Osmium::Ser::TagList& tags) const {
                print_title("TAGS", tags);
                for (Osmium::Ser::TagList::iterator it = tags.begin(); it != tags.end(); ++it) {
                    m_out << m_prefix << "  k=|" << it->key() << "| v=|" << it->value() << "|" << "\n";
                }
            }

            void print_way_node_list(const Osmium::Ser::WayNodeList& wnl) const {
                print_title("NODES", wnl);
                for (Osmium::Ser::WayNodeList::iterator it = wnl.begin(); it != wnl.end(); ++it) {
                    m_out << m_prefix << "  ref=" << it->ref() << "\n";
                }
            }

            void print_way_node_list_with_position(const Osmium::Ser::WayNodeWithPositionList& wnl) const {
                print_title("NODES", wnl);
                for (Osmium::Ser::WayNodeWithPositionList::iterator it = wnl.begin(); it != wnl.end(); ++it) {
                    m_out << m_prefix << "  ref=" << it->ref() << " pos=" << it->position() << "\n";
                }
            }
            
            void print_relation_member_list(const Osmium::Ser::RelationMemberList& rml) const {
                print_title("MEMBERS", rml);
                for (Osmium::Ser::RelationMemberList::iterator it = rml.begin(); it != rml.end(); ++it) {
                    m_out << m_prefix << "  type=" << it->type() << " ref=" << it->ref() << " role=|" << it->role() << "|\n";
                    if (it->full_member()) {
                        Dump dump(m_out, m_with_size, m_prefix + "  | ");
                        dump(it->get_object());
                    }
                }
            }

            void operator()(const Osmium::Ser::TypedItem& item) const {
                switch (item.type().t()) {
                    case Osmium::Ser::ItemType::itemtype_node:
                        print_title("NODE", item);
                        print_meta(static_cast<const Osmium::Ser::Object&>(item));
                        print_position(static_cast<const Osmium::Ser::Node&>(item));
                        break;
                    case Osmium::Ser::ItemType::itemtype_way:
                        print_title("WAY", item);
                        print_meta(static_cast<const Osmium::Ser::Object&>(item));
                        break;
                    case Osmium::Ser::ItemType::itemtype_relation:
                        print_title("RELATION", item);
                        print_meta(static_cast<const Osmium::Ser::Object&>(item));
                        break;
                    case Osmium::Ser::ItemType::itemtype_tag_list:
                        print_tag_list(static_cast<const Osmium::Ser::TagList&>(item));
                        break;
                    case Osmium::Ser::ItemType::itemtype_way_node_list:
                        print_way_node_list(static_cast<const Osmium::Ser::WayNodeList&>(item));
                        break;
                    case Osmium::Ser::ItemType::itemtype_way_node_with_position_list:
                        print_way_node_list_with_position(static_cast<const Osmium::Ser::WayNodeWithPositionList&>(item));
                        break;
                    case Osmium::Ser::ItemType::itemtype_relation_member_list:
                    case Osmium::Ser::ItemType::itemtype_relation_member_list_with_full_members:
                        print_relation_member_list(static_cast<const Osmium::Ser::RelationMemberList&>(item));
                        break;
                    default:
                        print_title("UNKNOWN", item);
                }
            }

        private:

            std::ostream& m_out;
            bool m_with_size;
            std::string m_prefix;

        }; // class Dump

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_DEBUG_HPP
