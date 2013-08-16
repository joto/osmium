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
#include <iostream>

#include <osmium/handler.hpp>

namespace Osmium {

    namespace Handler {

        /**
         * This handler dumps information about each callback and about each
         * object to stdout.
         */
        class Debug : public Base {

        public:

            Debug(bool has_multiple_object_versions=false, std::ostream &output_stream = std::cout) :
                Base(),
                m_has_multiple_object_versions(has_multiple_object_versions),
                m_output_stream(output_stream) {
            }

            void init(Osmium::OSM::Meta& meta) {
                m_output_stream << "meta:\n  generator=" << meta.generator() << "\n";
                if (meta.has_multiple_object_versions()) {
                    m_has_multiple_object_versions = true;
                }

                if (meta.bounds().defined()) {
                    m_output_stream << "  bounds=" << meta.bounds() << "\n";
                }
            }

            void before_nodes() const {
                m_output_stream << "before_nodes\n";
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) const {
                m_output_stream << "node:\n";
                print_meta(node);
                const Osmium::OSM::Position& position = node->position();
                m_output_stream << "  lon=" << std::fixed << std::setprecision(7) << position.lon() << "\n";
                m_output_stream << "  lat=" << std::fixed << std::setprecision(7) << position.lat() << "\n";
            }

            void after_nodes() const {
                m_output_stream << "after_nodes\n";
            }

            void before_ways() const {
                m_output_stream << "before_ways\n";
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& way) const {
                m_output_stream << "way:\n";
                print_meta(way);
                m_output_stream << "  node_count=" << way->nodes().size() << "\n";
                m_output_stream << "  nodes:\n";
                Osmium::OSM::WayNodeList::const_iterator end = way->nodes().end();
                for (Osmium::OSM::WayNodeList::const_iterator it = way->nodes().begin(); it != end; ++it) {
                    m_output_stream << "    ref=" << it->ref() << "\n";
                }
            }

            void after_ways() const {
                m_output_stream << "after_ways\n";
            }

            void before_relations() const {
                m_output_stream << "before_relations\n";
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) const {
                m_output_stream << "relation:\n";
                print_meta(relation);
                m_output_stream << "  members: (count=" << relation->members().size() << ")\n";
                Osmium::OSM::RelationMemberList::const_iterator end = relation->members().end();
                for (Osmium::OSM::RelationMemberList::const_iterator it = relation->members().begin(); it != end; ++it) {
                    m_output_stream << "    type=" << it->type() << " ref=" << it->ref() << " role=|" << it->role() << "|" << "\n";
                }
            }

            void after_relations() const {
                m_output_stream << "after_relations\n";
            }

            void final() const {
                m_output_stream << "final\n";
            }

        private:

            bool m_has_multiple_object_versions;
            std::ostream& m_output_stream;

            void print_meta(const shared_ptr<Osmium::OSM::Object const>& object) const {
                m_output_stream <<   "  id="        << object->id()
                          << "\n  version="   << object->version()
                          << "\n  uid="       << object->uid()
                          << "\n  user=|"     << object->user() << "|"
                          << "\n  changeset=" << object->changeset()
                          << "\n  timestamp=" << object->timestamp_as_string();
                if (m_has_multiple_object_versions) {
                    m_output_stream << "\n  visible=" << (object->visible() ? "yes" : "no")
                              << "\n  endtime=" << object->endtime_as_string();
                }
                m_output_stream << "\n  tags: (count=" << object->tags().size() << ")\n";
                Osmium::OSM::TagList::const_iterator end = object->tags().end();
                for (Osmium::OSM::TagList::const_iterator it = object->tags().begin(); it != end; ++it) {
                    m_output_stream << "    k=|" << it->key() << "| v=|" << it->value() << "|" << "\n";
                }
            }

        }; // class Debug

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_DEBUG_HPP
