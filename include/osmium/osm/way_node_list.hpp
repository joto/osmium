#ifndef OSMIUM_OSM_WAY_NODE_LIST_HPP
#define OSMIUM_OSM_WAY_NODE_LIST_HPP

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

            WayNodeList(unsigned int size=default_size) : m_list() {
                m_list.reserve(size);
            }

            osm_sequence_id_t size() const {
                return m_list.size();
            }

            void clear() {
                m_list.clear();
            }

            typedef std::vector<WayNode>::iterator iterator;
            typedef std::vector<WayNode>::const_iterator const_iterator;
            typedef std::vector<WayNode>::reverse_iterator reverse_iterator;
            typedef std::vector<WayNode>::const_reverse_iterator const_reverse_iterator;

            const_iterator begin() const {
                return m_list.begin();
            }

            const_iterator end() const {
                return m_list.end();
            }

            const_reverse_iterator rbegin() const {
                return m_list.rbegin();
            }

            const_reverse_iterator rend() const {
                return m_list.rend();
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

            const WayNode& back() const {
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

            WayNodeList& add(WayNode& way_node) {
                m_list.push_back(way_node);
                return *this;
            }

            WayNodeList& add(osm_object_id_t ref) {
                m_list.push_back(WayNode(ref));
                return *this;
            }

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_instance() const {
                return JavascriptTemplate::get<JavascriptTemplate>().create_instance((void *)this);
            }

            v8::Handle<v8::Value> js_length() const {
                return v8::Number::New(m_list.size());
            }

            v8::Handle<v8::Value> js_get_node_id(uint32_t index) const {
                if (sizeof(osm_object_id_t) <= 4) // XXX do this at compile time
                    return v8::Integer::New(m_list[index].ref());
                else
                    return v8::Number::New(m_list[index].ref());
            }

            v8::Handle<v8::Array> js_enumerate_nodes() const {
                v8::Local<v8::Array> array = v8::Array::New(m_list.size());

                for (osm_sequence_id_t i=0; i < m_list.size(); i++) {
                    v8::Local<v8::Integer> ii = v8::Integer::New(i);
                    array->Set(ii, ii);
                }

                return array;
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template {

                JavascriptTemplate() : Osmium::Javascript::Template() {
                    js_template->SetAccessor(v8::String::NewSymbol("length"), accessor_getter<WayNodeList, &WayNodeList::js_length>);
                    js_template->SetIndexedPropertyHandler(
                        indexed_property_getter<WayNodeList, &WayNodeList::js_get_node_id>,
                        0,
                        0,
                        0,
                        property_enumerator<WayNodeList, &WayNodeList::js_enumerate_nodes>
                    );
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        private:

            std::vector<WayNode> m_list;

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> m_js_instance;
#endif // OSMIUM_WITH_JAVASCRIPT

        }; // class WayNodeList

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_WAY_NODE_LIST_HPP
