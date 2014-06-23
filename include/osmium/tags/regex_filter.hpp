#ifndef OSMIUM_TAGS_REGEX_FILTER_HPP
#define OSMIUM_TAGS_REGEX_FILTER_HPP

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

#define OSMIUM_LINK_WITH_LIBS_REGEX -lboost_regex

#include <functional>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/regex.hpp>

#include <osmium/osm/tag.hpp>
#include <osmium/osm/tag_list.hpp>

namespace Osmium {

    namespace Tags {

        class RegexFilter : public std::unary_function<const Osmium::OSM::Tag&, bool> {

            struct rule_t {
                bool result;
                boost::regex key;
                boost::regex value;

                rule_t(bool r, const char* k, const char* v) :
                    result(r),
                    key(k),
                    value() {
                    if (v) {
                        value = v;
                    }
                }

            };

            std::vector<rule_t> m_rules;
            bool m_default_result;

        public:

            typedef boost::filter_iterator<RegexFilter, Osmium::OSM::TagList::const_iterator> iterator;

            RegexFilter(bool default_result) :
                m_rules(),
                m_default_result(default_result) {
            }

            RegexFilter& add(bool result, const char* key, const char* value = NULL) {
                m_rules.push_back(rule_t(result, key, value));
                return *this;
            }

            bool operator()(const Osmium::OSM::Tag& tag) const {
                BOOST_FOREACH(const rule_t &rule, m_rules) {
                    if (boost::regex_match(tag.key(), rule.key) && (rule.value.empty() || boost::regex_match(tag.value(), rule.value))) {
                        return rule.result;
                    }
                }
                return m_default_result;
            }

        };

    } // namespace Tags

} // namespace Osmium

#endif // OSMIUM_TAGS_REGEX_FILTER_HPP
