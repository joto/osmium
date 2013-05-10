#ifndef OSMIUM_SER_ITEM_HPP
#define OSMIUM_SER_ITEM_HPP

/*

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>

namespace Osmium {

    namespace Ser {

        // sizes and offsets inside buffer
        typedef uint64_t length_t;

        inline length_t padded_length(length_t length) {
            return (length % 8 == 0) ? length : ((length | 7 ) + 1);
        }

        // any kind of item in a buffer
        class Item {

        public:
        
            uint64_t offset;
            char type;
            char padding[7];

            Item() : offset(0), type('-') {
            }

        protected:

            const char* self() const {
                return reinterpret_cast<const char*>(this);
            }

        };

        // serialized form of OSM object
        class Object : public Item {
        public:

            uint64_t id;
            uint64_t version;
            uint32_t timestamp;
            uint32_t uid;
            uint64_t changeset;

            const char* user_position() const {
                return self() + sizeof(Object) + (type == 'n' ? sizeof(Osmium::OSM::Position) : 0);
            }

            const char* user() const {
                return user_position() + sizeof(length_t);
            }

            length_t user_length() const {
                return *reinterpret_cast<const length_t*>(user_position());
            }

            const char* tags_position() const {
                return user_position() + sizeof(length_t) + padded_length(user_length());
            }

            length_t tags_length() const {
                return *reinterpret_cast<const length_t*>(tags_position());
            }

            const char* members_position() const {
                return tags_position() + sizeof(length_t) + padded_length(tags_length());
            }

            length_t members_length() const {
                return *reinterpret_cast<const length_t*>(members_position());
            }

        };

        // serialized form of OSM node
        class Node : public Object {

        public:

            static char object_type() {
                return 'n';
            }

            Osmium::OSM::Position pos;

        }; // class Node

        class Way : public Object {

        public:

            static char object_type() {
                return 'w';
            }

        }; // class Way

        class Relation : public Object {

        public:

            static char object_type() {
                return 'r';
            }

        }; // class Relation


        class RelationMember {

        public:

            osm_object_id_t ref;
            char type;
            char tpadding[7];

            RelationMember() : ref(0), type('-') {
                memset(tpadding, 0, 7);
            }

            const char* role_position() const {
                return self() + sizeof(RelationMember);
            }

            const char* role() const {
                return role_position() + sizeof(length_t);
            }

            const char* self() const {
                return reinterpret_cast<const char*>(this);
            }

        }; // class RelationMember

        /**
         * Iterator to iterate over tags in a RelationMembers
         */
        class RelationMembersIter {

        public:

            RelationMembersIter(const char* start, const char* end) : m_start(start), m_end(end) {
            }

            RelationMembersIter& operator++() {
                m_start += sizeof(RelationMember);
                m_start += padded_length(*reinterpret_cast<const length_t*>(m_start)) + sizeof(length_t);
                return *this;
            }

            RelationMembersIter operator++(int) {
                RelationMembersIter tmp(*this);
                operator++();
                return tmp;
            }

            bool operator==(const RelationMembersIter& rhs) {return m_start==rhs.m_start;}
            bool operator!=(const RelationMembersIter& rhs) {return m_start!=rhs.m_start;}

            const RelationMember operator*() {
                return *reinterpret_cast<const RelationMember*>(m_start);
            }
           
            const RelationMember* operator->() {
                return reinterpret_cast<const RelationMember*>(m_start);
            }

        private:

            const char* m_start;
            const char* m_end;

        }; // class RelationMembersIter

        class RelationMembers {

        public:

            RelationMembers(const char* data, size_t size) : m_data(data), m_size(size) {
            }

            RelationMembers(const Object& object) : m_data(object.members_position() + sizeof(length_t)), m_size(object.members_length()) {
            }

            RelationMembersIter begin() {
                return RelationMembersIter(m_data, m_data + m_size);
            }

            RelationMembersIter end() {
                return RelationMembersIter(m_data + m_size, m_data + m_size);
            }

        private:

            const char* m_data;
            size_t m_size;

        }; // class RelationMembers

        class Tag {

        public:

            Tag(const char* data) : m_data(data) {
            }

            const char* key() const {
                return m_data;
            }

            const char* value() const {
                return m_data + strlen(m_data) + 1;
            }

        private:

            const char* m_data;

        }; // class Tag

        /**
         * Iterator to iterate over tags in a Tags
         */
        class TagsIter {

        public:

            TagsIter(const char* start, const char* end) : m_start(start), m_end(end) {
            }

            TagsIter& operator++() {
                m_start += strlen(m_start) + 1;
                m_start += strlen(m_start) + 1;
                return *this;
            }

            TagsIter operator++(int) {
                TagsIter tmp(*this);
                operator++();
                return tmp;
            }

            bool operator==(const TagsIter& rhs) {return m_start==rhs.m_start;}
            bool operator!=(const TagsIter& rhs) {return m_start!=rhs.m_start;}

            const Tag operator*() {
                return Tag(m_start);
            }
            
            const Tag* operator->() {
                return reinterpret_cast<const Tag*>(&m_start);
            }

        private:

            const char* m_start;
            const char* m_end;

        }; // class TagsIter

        class Tags {

        public:

            Tags(const char* data, size_t size) : m_data(data), m_size(size) {
            }

            Tags(const Object& object) : m_data(object.tags_position() + sizeof(length_t)), m_size(object.tags_length()) {
            }

            TagsIter begin() {
                return TagsIter(m_data, m_data + m_size);
            }

            TagsIter end() {
                return TagsIter(m_data + m_size, m_data + m_size);
            }

        private:

            const char* m_data;
            size_t m_size;

        }; // class Tags

        /**
         * Iterator to iterate over nodes in Nodes
         */
        class NodesIter {

        public:

            NodesIter(const char* start, const char* end) : m_start(start), m_end(end) {
            }

            NodesIter& operator++() {
                m_start += sizeof(uint64_t);
                return *this;
            }

            NodesIter operator++(int) {
                NodesIter tmp(*this);
                operator++();
                return tmp;
            }

            bool operator==(const NodesIter& rhs) {return m_start==rhs.m_start;}
            bool operator!=(const NodesIter& rhs) {return m_start!=rhs.m_start;}

            uint64_t operator*() {
                return *reinterpret_cast<const uint64_t*>(m_start);
            }
            
        private:

            const char* m_start;
            const char* m_end;

        }; // class NodesIter

        /**
         * List of nodes in a buffer.
         */
        class Nodes {

        public:

            Nodes(const char* data) : m_data(data+sizeof(length_t)), m_size(*reinterpret_cast<const length_t*>(data)) {
            }

            Nodes(const Osmium::Ser::Way& way) : m_data(way.members_position() + sizeof(length_t)), m_size(way.members_length()) {
            }

            NodesIter begin() {
                return NodesIter(m_data, m_data + m_size);
            }

            NodesIter end() {
                return NodesIter(m_data + m_size, m_data + m_size);
            }

        private:

            const char* m_data;
            int32_t m_size;

        }; // class Nodes

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_ITEM_HPP
