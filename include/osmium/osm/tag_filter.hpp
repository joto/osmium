#ifndef OSMIUM_OSM_TAG_FILTER_HPP
#define OSMIUM_OSM_TAG_FILTER_HPP

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

#include <functional>
#include <numeric>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <osmium/osm/tag.hpp>
#include <osmium/osm/tag_list.hpp>

namespace Osmium {

    namespace OSM {

        class TagKeyFilterOp : public std::unary_function<const Osmium::OSM::Tag&, bool> {

        public:

            TagKeyFilterOp() :
                m_keys() {
            }

            TagKeyFilterOp& add(const char* key) {
                m_keys.push_back(key);
                return *this;
            }

            bool operator()(const Osmium::OSM::Tag& tag) const {
                BOOST_FOREACH(const std::string& key, m_keys) {
                    if (key == tag.key()) {
                        return false;
                    }
                }
                return true;
            }

        private:

            std::vector<std::string> m_keys;

        };

        typedef boost::filter_iterator<TagKeyFilterOp, TagList::const_iterator> TagKeyFilterOpIterator;

        template <class T>
        inline void tags_filter_and_accumulate(const TagList& tags, TagKeyFilterOp& filter, std::string* stringptr, T convert) {
            TagKeyFilterOpIterator fi_begin(filter, tags.begin(), tags.end());
            TagKeyFilterOpIterator fi_end(filter, tags.end(), tags.end());

            std::accumulate(fi_begin, fi_end, stringptr, convert);
        }

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_TAG_FILTER_HPP
