#ifndef OSMIUM_OSM_TAG_LIST_HPP
#define OSMIUM_OSM_TAG_LIST_HPP

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
#include <cstring>
#include <stdexcept>

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

            TagList() : m_tags() {
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
            void add(const char *key, const char *value) {
                m_tags.push_back(Tag(key, value));
            }

            const char *get_tag_by_key(const char *key) const {
                for (const_iterator it = begin(); it != end(); ++it) {
                    if (!strcmp(it->key(), key)) {
                        return it->value();
                    }
                }
                return 0;
            }

            const char *get_tag_key(unsigned int n) const {
                if (n < m_tags.size()) {
                    return m_tags[n].key();
                }
                throw std::range_error("no tag with this index");
            }

            const char *get_tag_value(unsigned int n) const {
                if (n < m_tags.size()) {
                    return m_tags[n].value();
                }
                throw std::range_error("no tag with this index");
            }

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Local<v8::Object> js_instance() const {
                return JavascriptTemplate::get<JavascriptTemplate>().create_instance((void *)this);
            }

            v8::Handle<v8::Value> js_get_tag_value_by_key(v8::Local<v8::String> property) const {
                const char *key = Osmium::v8_String_to_utf8<Osmium::OSM::Tag::max_utf16_length_key>(property);
                const char *value = get_tag_by_key(key);
                if (value) {
                    return Osmium::utf8_to_v8_String<Osmium::OSM::Tag::max_utf16_length_value>(value);
                }
                return v8::Undefined();
            }

            v8::Handle<v8::Array> js_enumerate_tag_keys() const {
                v8::HandleScope scope;
                v8::Local<v8::Array> array = v8::Array::New(m_tags.size());

                const_iterator end = this->end();
                int i = 0;
                for (const_iterator it = begin(); it != end; ++it) {
                    array->Set(i++, Osmium::utf8_to_v8_String<Osmium::OSM::Tag::max_utf16_length_key>(it->key()));
                }

                return scope.Close(array);
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template {

                JavascriptTemplate() : Osmium::Javascript::Template() {
                    js_template->SetNamedPropertyHandler(
                        named_property_getter<TagList, &TagList::js_get_tag_value_by_key>,
                        0,
                        0,
                        0,
                        property_enumerator<TagList, &TagList::js_enumerate_tag_keys>
                    );
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

        private:

            std::vector<Tag> m_tags;

        }; // class TagList

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_TAG_LIST_HPP
