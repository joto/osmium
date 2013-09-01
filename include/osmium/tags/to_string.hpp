#ifndef OSMIUM_TAGS_TAG_LIST_TO_STRING_HPP
#define OSMIUM_TAGS_TAG_LIST_TO_STRING_HPP

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
#include <string>

#include <osmium/osm/tag.hpp>

namespace Osmium {

    namespace Tags {

        /**
         * Operation that turns a Tag into a string with many parameters that define how this is done.
         *
         * @param escape String that contains all characters that should be escaped with a backslash (\)
         * @param prefix String printed before a tag.
         * @param infix String printed between key and value of a tag.
         * @param suffix String printed after a tag.
         * @param join String used to join several tags together.
         */
        class TagToStringOp : public std::binary_function<std::string&, const Osmium::OSM::Tag&, std::string&> {

        public:

            TagToStringOp(const std::string& escape, const char* prefix, const char* infix, const char* suffix, const char* join) :
                m_escape(escape),
                m_prefix(prefix),
                m_infix(infix),
                m_suffix(suffix),
                m_join(join) {
            }

            std::string& operator()(std::string& output, const Osmium::OSM::Tag& tag) const {
                if (!output.empty()) {
                    output.append(m_join);
                }
                output.append(m_prefix);
                append_escaped_string(output, tag.key());
                output.append(m_infix);
                append_escaped_string(output, tag.value());
                output.append(m_suffix);
                return output;
            }

        private:

            const std::string m_escape;
            const char* m_prefix;
            const char* m_infix;
            const char* m_suffix;
            const char* m_join;

            void append_escaped_string(std::string& output, const char* in) const {
                while (*in) {
                    if (m_escape.find(*in) != std::string::npos) {
                        output.append(1, '\\');
                    }
                    output.append(1, *in++);
                }
            }

        }; // class TagToStringOp

        /**
         * Operation that turns a Tag into a string in the format "key=value".
         */
        class TagToKeyEqualsValueStringOp : public TagToStringOp {

        public:

            TagToKeyEqualsValueStringOp(const char* join) :
                TagToStringOp("", "", "=", "", join) {
            }

        }; // class TagToKeyEqualsValueStringOp

        /**
         * Operation that turns a Tag into a string in the format used for the hstore PostgreSQL extension.
         */
        class TagToHStoreStringOp : public TagToStringOp {

        public:

            TagToHStoreStringOp() :
                TagToStringOp("\\\"", "\"", "\"=>\"", "\"", ",") {
            }

        }; // class TagToHStoreStringOp

    } // namespace Tags

} // namespace Osmium

#endif // OSMIUM_TAGS_TAG_LIST_TO_STRING_HPP
