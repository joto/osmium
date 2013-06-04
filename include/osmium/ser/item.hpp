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

        inline size_t padded_length(size_t length) {
            return (length % 8 == 0) ? length : ((length | 7 ) + 1);
        }

        class ItemType {

        public:

            static const uint32_t itemtype_unknown  = 0;
            static const uint32_t itemtype_node     = 1;
            static const uint32_t itemtype_way      = 2;
            static const uint32_t itemtype_relation = 3;

            ItemType(uint32_t type = itemtype_unknown) : m_type(type) {
            }

            ItemType(char c) : m_type(itemtype_unknown) {
                switch (c) {
                    case 'n': m_type = itemtype_node; break;
                    case 'w': m_type = itemtype_way; break;
                    case 'r': m_type = itemtype_relation; break;
                    default:  m_type = itemtype_unknown;
                }
            }

            bool is_node() const {
                return m_type == itemtype_node;
            }

            bool is_way() const {
                return m_type == itemtype_way;
            }

            bool is_relation() const {
                return m_type == itemtype_relation;
            }

            char as_char() const {
                switch (m_type) {
                    case itemtype_node: return 'n';
                    case itemtype_way: return 'w';
                    case itemtype_relation: return 'r';
                }
                return '-';
            }

        private:

            uint32_t m_type;

        };

        std::ostream& operator<<(std::ostream& out, const ItemType type) {
            out << type.as_char();
            return out;
        }

        template <class T>
        inline const ItemType itemtype_of() {
            return ItemType(ItemType::itemtype_unknown);
        }

        // any kind of item in a buffer
        class Item {

        public:
        
            uint64_t offset;
            ItemType type;
            char padding[4];

            Item() : offset(0), type() {
                memset(padding, 0, 4);
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
                return self() + sizeof(Object) + (type.is_node() ? sizeof(Osmium::OSM::Position) : 0);
            }

            const char* user() const {
                return user_position() + sizeof(size_t);
            }

            size_t user_length() const {
                return *reinterpret_cast<const size_t*>(user_position());
            }

            const char* tags_position() const {
                return user_position() + sizeof(size_t) + padded_length(user_length());
            }

            size_t tags_length() const {
                return *reinterpret_cast<const size_t*>(tags_position());
            }

            const char* members_position() const {
                return tags_position() + sizeof(size_t) + padded_length(tags_length());
            }

            size_t members_length() const {
                return *reinterpret_cast<const size_t*>(members_position());
            }

        };

        // serialized form of OSM node
        class Node : public Object {

        public:

            Osmium::OSM::Position pos;

        }; // class Node

        template <>
        inline const ItemType itemtype_of<Node>() {
            return ItemType(ItemType::itemtype_node);
        }

        class Way : public Object {
        }; // class Way

        template <>
        inline const ItemType itemtype_of<Way>() {
            return ItemType(ItemType::itemtype_way);
        }

        class Relation : public Object {
        }; // class Relation

        template <>
        inline const ItemType itemtype_of<Relation>() {
            return ItemType(ItemType::itemtype_relation);
        }

        class RelationMember {

        public:

            osm_object_id_t ref;
            ItemType type;
            char tpadding[4];

            RelationMember() : ref(0), type() {
                memset(tpadding, 0, 4);
            }

            const char* role_position() const {
                return self() + sizeof(RelationMember);
            }

            const char* role() const {
                return role_position() + sizeof(size_t);
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
                m_start += padded_length(*reinterpret_cast<const size_t*>(m_start)) + sizeof(size_t);
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

            RelationMembers(const Object& object) : m_data(object.members_position() + sizeof(size_t)), m_size(object.members_length()) {
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

            Tags(const Object& object) : m_data(object.tags_position() + sizeof(size_t)), m_size(object.tags_length()) {
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

            Nodes(const char* data) : m_data(data+sizeof(size_t)), m_size(*reinterpret_cast<const size_t*>(data)) {
            }

            Nodes(const Osmium::Ser::Way& way) : m_data(way.members_position() + sizeof(size_t)), m_size(way.members_length()) {
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
