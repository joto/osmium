#ifndef OSMIUM_OSM_TAG_LIST_HPP
#define OSMIUM_OSM_TAG_LIST_HPP

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

#include <cstring>
#include <stdexcept>
#include <vector>

#include <osmium/osm/tag.hpp>

namespace Osmium {

    namespace OSM {

        /**
        * An ordered container for Tags.
        *
        * Tag keys are assumed to be unique in a TagList, but this is not
        * checked.
        */
        class TagList {

        public:

            TagList() :
                m_tags() {
            }

            /// Return the number of tags in this tag list.
            int size() const {
                return m_tags.size();
            }

            bool empty() const {
                return m_tags.empty();
            }

            /// Remove all tags from the tag list.
            void clear() {
                m_tags.clear();
            }

            Tag& operator[](int i) {
                return m_tags[i];
            }

            const Tag& operator[](int i) const {
                return m_tags[i];
            }

            typedef std::vector<Tag>::iterator iterator;
            typedef std::vector<Tag>::const_iterator const_iterator;

            iterator begin() {
                return m_tags.begin();
            }

            const_iterator begin() const {
                return m_tags.begin();
            }

            iterator end() {
                return m_tags.end();
            }

            const_iterator end() const {
                return m_tags.end();
            }

            /// Add new tag with given key and value to list.
            void add(const char* key, const char* value) {
                m_tags.push_back(Tag(key, value));
            }

            const char* get_value_by_key(const char* key) const {
                for (const_iterator it = begin(); it != end(); ++it) {
                    if (!strcmp(it->key(), key)) {
                        return it->value();
                    }
                }
                return 0;
            }

        private:

            std::vector<Tag> m_tags;

        }; // class TagList

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_TAG_LIST_HPP
