#ifndef OSMIUM_HANDLER_DEBUG_HPP
#define OSMIUM_HANDLER_DEBUG_HPP

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

#include <iomanip>

#include <osmium/handler.hpp>

namespace Osmium {

    namespace Handler {

        /**
         * This handler dumps information about each callback and about each
         * object to stdout.
         */
        class Debug : public Base {

        public:

            Debug(bool has_multiple_object_versions=false) :
                Base(),
                m_has_multiple_object_versions(has_multiple_object_versions) {
            }

            void init(Osmium::OSM::Meta& meta) {
                std::cout << "meta:\n";
                if (meta.has_multiple_object_versions()) {
                    m_has_multiple_object_versions = true;
                }

                if (meta.bounds().defined()) {
                    std::cout << "  bounds=" << meta.bounds() << "\n";
                }
            }

            void before_nodes() const {
                std::cout << "before_nodes\n";
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) const {
                std::cout << "node:\n";
                print_meta(node);
                const Osmium::OSM::Position& position = node->position();
                std::cout << "  lon=" << std::fixed << std::setprecision(7) << position.lon() << "\n";
                std::cout << "  lat=" << std::fixed << std::setprecision(7) << position.lat() << "\n";
            }

            void after_nodes() const {
                std::cout << "after_nodes\n";
            }

            void before_ways() const {
                std::cout << "before_ways\n";
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& way) const {
                std::cout << "way:\n";
                print_meta(way);
                std::cout << "  node_count=" << way->nodes().size() << "\n";
                std::cout << "  nodes:\n";
                Osmium::OSM::WayNodeList::const_iterator end = way->nodes().end();
                for (Osmium::OSM::WayNodeList::const_iterator it = way->nodes().begin(); it != end; ++it) {
                    std::cout << "    ref=" << it->ref() << "\n";
                }
            }

            void after_ways() const {
                std::cout << "after_ways\n";
            }

            void before_relations() const {
                std::cout << "before_relations\n";
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) const {
                std::cout << "relation:\n";
                print_meta(relation);
                std::cout << "  members: (count=" << relation->members().size() << ")\n";
                Osmium::OSM::RelationMemberList::const_iterator end = relation->members().end();
                for (Osmium::OSM::RelationMemberList::const_iterator it = relation->members().begin(); it != end; ++it) {
                    std::cout << "    type=" << it->type() << " ref=" << it->ref() << " role=|" << it->role() << "|" << "\n";
                }
            }

            void after_relations() const {
                std::cout << "after_relations\n";
            }

            void final() const {
                std::cout << "final\n";
            }

        private:

            bool m_has_multiple_object_versions;

            void print_meta(const shared_ptr<Osmium::OSM::Object const>& object) const {
                std::cout <<   "  id="        << object->id()
                          << "\n  version="   << object->version()
                          << "\n  uid="       << object->uid()
                          << "\n  user=|"     << object->user() << "|"
                          << "\n  changeset=" << object->changeset()
                          << "\n  timestamp=" << object->timestamp_as_string();
                if (m_has_multiple_object_versions) {
                    std::cout << "\n  visible=" << (object->visible() ? "yes" : "no")
                              << "\n  endtime=" << object->endtime_as_string();
                }
                std::cout << "\n  tags: (count=" << object->tags().size() << ")\n";
                Osmium::OSM::TagList::const_iterator end = object->tags().end();
                for (Osmium::OSM::TagList::const_iterator it = object->tags().begin(); it != end; ++it) {
                    std::cout << "    k=|" << it->key() << "| v=|" << it->value() << "|" << "\n";
                }
            }

        }; // class Debug

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_DEBUG_HPP
