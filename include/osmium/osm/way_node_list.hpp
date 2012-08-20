#ifndef OSMIUM_OSM_WAY_NODE_LIST_HPP
#define OSMIUM_OSM_WAY_NODE_LIST_HPP

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

#include <vector>

#include <osmium/osm/way_node.hpp>

namespace Osmium {

    namespace OSM {

        class WayNodeList {

        public:

            /**
             * If a WayNodeList object is created and the number of nodes is
             * not given to the constructor, space for this many nodes is
             * reserved. 99.9% of all ways have 500 or less nodes.
             */
            static const int default_size = 500;

            WayNodeList(unsigned int size=default_size) :
                m_list() {
                m_list.reserve(size);
            }

            osm_sequence_id_t size() const {
                return m_list.size();
            }

            bool empty() const {
                return m_list.empty();
            }

            void clear() {
                m_list.clear();
            }

            typedef std::vector<WayNode>::iterator iterator;
            typedef std::vector<WayNode>::const_iterator const_iterator;
            typedef std::vector<WayNode>::reverse_iterator reverse_iterator;
            typedef std::vector<WayNode>::const_reverse_iterator const_reverse_iterator;

            iterator begin() {
                return m_list.begin();
            }

            const_iterator begin() const {
                return m_list.begin();
            }

            iterator end() {
                return m_list.end();
            }

            const_iterator end() const {
                return m_list.end();
            }

            reverse_iterator rbegin() {
                return m_list.rbegin();
            }

            const_reverse_iterator rbegin() const {
                return m_list.rbegin();
            }

            reverse_iterator rend() {
                return m_list.rend();
            }

            const_reverse_iterator rend() const {
                return m_list.rend();
            }

            template <class TInputIterator>
            void insert(iterator position, TInputIterator first, TInputIterator last) {
                m_list.insert(position, first, last);
            }

            WayNode& operator[](int i) {
                return m_list[i];
            }

            const WayNode& operator[](int i) const {
                return m_list[i];
            }

            const WayNode& front() const {
                return m_list.front();
            }

            WayNode& front() {
                return m_list.front();
            }

            const WayNode& back() const {
                return m_list.back();
            }

            WayNode& back() {
                return m_list.back();
            }

            bool is_closed() const {
                return m_list.front().ref() == m_list.back().ref();
            }

            bool has_position() const {
                if (m_list.empty()) {
                    return false;
                } else {
                    return m_list.back().has_position();
                }
            }

            WayNodeList& push_back(const WayNode& way_node) {
                m_list.push_back(way_node);
                return *this;
            }

            WayNodeList& add(const WayNode& way_node) {
                m_list.push_back(way_node);
                return *this;
            }

            WayNodeList& push_back(osm_object_id_t ref) {
                m_list.push_back(WayNode(ref));
                return *this;
            }

            WayNodeList& add(osm_object_id_t ref) {
                m_list.push_back(WayNode(ref));
                return *this;
            }

        private:

            std::vector<WayNode> m_list;

        }; // class WayNodeList

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_WAY_NODE_LIST_HPP
