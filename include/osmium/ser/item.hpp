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

#include <osmium/ser/buffer.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>

namespace Osmium {

    namespace Ser {

        inline size_t padded_length(size_t length) {
            return (length + align_bytes - 1) & ~(align_bytes - 1);
        }

        class ItemType {

        public:

            static const uint32_t itemtype_unknown  = 0x00;
            static const uint32_t itemtype_node     = 0x01;
            static const uint32_t itemtype_way      = 0x02;
            static const uint32_t itemtype_relation = 0x03;
            static const uint32_t itemtype_taglist  = 0x10;

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
        struct item_traits {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_unknown;
        };

        // any kind of item in a buffer
        class Item {

        public:
        
            Item(size_t size=0, ItemType type=ItemType()) :
                m_size(size),
                m_type(type) {
            }

            void add_size(uint32_t size) {
                m_size += size;
            }

            uint32_t size() const {
                return m_size;
            }

            void type(const ItemType& item_type) {
                m_type = item_type;
            }

            ItemType type() const {
                return m_type;
            }

        protected:

            const char* self() const {
                return reinterpret_cast<const char*>(this);
            }

        private:

            uint32_t m_size;
            ItemType m_type;

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
                return self() + sizeof(Object) + (type().is_node() ? sizeof(Osmium::OSM::Position) : 0);
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

            uint32_t tags_length() const {
                return reinterpret_cast<const Osmium::Ser::Item*>(tags_position())->size();
            }

            const char* members_position() const {
                return tags_position() + padded_length(tags_length());
            }

            uint32_t members_length() const {
                return reinterpret_cast<const Osmium::Ser::Item*>(members_position())->size();
            }

        };

        // serialized form of OSM node
        class Node : public Object {

        public:

            Osmium::OSM::Position pos;

        }; // class Node

        template <>
        struct item_traits<Node> {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_node;
        };


        class Way : public Object {
        }; // class Way

        template <>
        struct item_traits<Way> {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_way;
        };


        class Relation : public Object {
        }; // class Relation

        template <>
        struct item_traits<Relation> {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_relation;
        };


        template <class TMember>
        class CollectionIterator {

        public:

            CollectionIterator(const char* start, const char* end) :
                m_start(start),
                m_end(end) {
            }

            CollectionIterator<TMember>& operator++() {
                m_start = reinterpret_cast<const TMember*>(m_start)->next();
                return *static_cast<CollectionIterator<TMember>*>(this);
            }

            CollectionIterator<TMember> operator++(int) {
                CollectionIterator<TMember> tmp(*this);
                operator++();
                return tmp;
            }

            bool operator==(const CollectionIterator<TMember>& rhs) const {
                return m_start == rhs.m_start;
            }

            bool operator!=(const CollectionIterator<TMember>& rhs) const {
                return m_start != rhs.m_start;
            }

            const TMember operator*() {
                return *reinterpret_cast<const TMember*>(m_start);
            }
            
            const TMember* operator->() {
                return reinterpret_cast<const TMember*>(m_start);
            }

        protected:

            const char*       m_start;
            const char* const m_end;

        }; // class CollectionIterator

        template <class TIter>
        class Collection : public Item {

        public:

            TIter begin() const {
                return TIter(self() + sizeof(Item), self() + size());
            }

            TIter end() const {
                return TIter(self() + size(), self() + size());
            }

        }; // class Collection

        class Tag {

        public:

            const char* key() const {
                return reinterpret_cast<const char*>(this);
            }

            const char* value() const {
                const char* data = reinterpret_cast<const char*>(this);
                return data + strlen(data) + 1;
            }

            const char* next() const {
                const char* current = reinterpret_cast<const char*>(this);
                current += strlen(current) + 1;
                return current + strlen(current) + 1;
            }

        }; // class Tag

        typedef CollectionIterator<Tag> TagsIter;
        typedef Collection<TagsIter> TagList;

        template <>
        struct item_traits<TagList> {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_taglist;
        };

        class WayNode {

        public:

            osm_object_id_t id() const {
                return m_id;
            }

            const char* next() const {
                return reinterpret_cast<const char*>(this + 1);
            }

            osm_object_id_t m_id;

        }; // class WayNode

        typedef CollectionIterator<WayNode> NodesIter;
        typedef Collection<NodesIter> NodeList;

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

            const char* next() const {
                const char* current = reinterpret_cast<const char*>(this + 1);
                return current + sizeof(size_t) + padded_length(*reinterpret_cast<const size_t*>(current));
            }

        }; // class RelationMember

        typedef CollectionIterator<RelationMember> RelationMembersIter;
        typedef Collection<RelationMembersIter> RelationMemberList;

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_ITEM_HPP
