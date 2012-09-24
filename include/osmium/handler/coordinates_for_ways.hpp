#ifndef OSMIUM_HANDLER_COORDINATES_FOR_WAYS_HPP
#define OSMIUM_HANDLER_COORDINATES_FOR_WAYS_HPP

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

#include <osmium/handler.hpp>

namespace Osmium {

    namespace Handler {

        /**
         * Handler to retrieve locations from nodes and add them to ways.
         *
         * @tparam TStorage Class that handles the actual storage of the node locations.
         *                  It must support the set(id, value) method and operator[] for
         *                  reading a value.
         */
        template <class TStoragePosIDs, class TStorageNegIDs>
        class CoordinatesForWays : public Base {

        public:

            CoordinatesForWays(TStoragePosIDs& storage_pos,
                               TStorageNegIDs& storage_neg) :
                m_storage_pos(storage_pos),
                m_storage_neg(storage_neg) {
            }

            /**
             * Store the location of the node in the storage.
             */
            void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                int64_t id = node->id();
                if (id >= 0) {
                    m_storage_pos.set(id, node->position());
                } else {
                    m_storage_neg.set(-id, node->position());
                }
            }

            Osmium::OSM::Position get_node_pos(const int64_t id) const {
                return id >= 0 ? m_storage_pos[id] : m_storage_neg[-id];
            }

            /**
             * Retrieve locations of all nodes in the way from storage and add
             * them to the way object.
             */
            void way(const shared_ptr<Osmium::OSM::Way>& way) {
                for (Osmium::OSM::WayNodeList::iterator it = way->nodes().begin(); it != way->nodes().end(); ++it) {
                    const int64_t id = it->ref();
                    it->position(id >= 0 ? m_storage_pos[id] : m_storage_neg[-id]);
                }
            }

        private:

            /// Object that handles the actual storage of the node locations (with positive IDs).
            TStoragePosIDs& m_storage_pos;

            /// Object that handles the actual storage of the node locations (with negative IDs).
            TStorageNegIDs& m_storage_neg;

        }; // class CoordinatesForWays

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_COORDINATES_FOR_WAYS_HPP
