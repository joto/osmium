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
#include <osmium/osm/types.hpp>
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
            static const uint32_t itemtype_collection                  = 0x10;
            static const uint32_t itemtype_tag_list                    = 0x11;
            static const uint32_t itemtype_way_node_list               = 0x12;
            static const uint32_t itemtype_way_node_with_position_list = 0x32;
            static const uint32_t itemtype_relation_member_list        = 0x13;
            static const uint32_t itemtype_relation_member_list_with_full_members = 0x33;

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

            // XXX ugh
            uint32_t t() const {
                return m_type;
            }

            bool equal(const ItemType& other) const {
                return m_type == other.m_type;
            }

        private:

            uint32_t m_type;

        };

        inline bool operator==(const ItemType& lhs, const ItemType& rhs) {
            return lhs.equal(rhs);
        }

        inline bool operator!=(const ItemType& lhs, const ItemType& rhs) {
            return !lhs.equal(rhs);
        }

        std::ostream& operator<<(std::ostream& out, const ItemType& type) {
            out << type.as_char();
            return out;
        }

        template <class T>
        struct item_traits {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_unknown;
        };

        // any kind of item in a buffer
        class Item {

        protected:

            Item() {}

            ~Item() {}

            const char* self() const {
                return reinterpret_cast<const char*>(this);
            }

        }; // class Item

        class TypedItem : public Item {

        public:
        
            TypedItem(size_t size=0, ItemType type=ItemType()) :
                Item(),
                m_size(size),
                m_type(type) {
            }

            void add_size(uint32_t size) {
                m_size += size;
            }

            uint32_t size() const {
                return m_size;
            }

            uint32_t padded_size() const {
                return (m_size + align_bytes - 1) & ~(align_bytes - 1);
            }

            void type(const ItemType& item_type) {
                m_type = item_type;
            }

            ItemType type() const {
                return m_type;
            }

        private:

            uint32_t m_size;
            ItemType m_type;

        }; // class TypedItem

        class SubItemIterator {

        public:

            SubItemIterator(const char* start, const char* end) :
                m_start(start),
                m_end(end) {
            }

            SubItemIterator& operator++() {
                m_start += reinterpret_cast<const TypedItem*>(m_start)->padded_size();
                return *this;
            }

            SubItemIterator operator++(int) {
                SubItemIterator tmp(*this);
                operator++();
                return tmp;
            }

            bool operator==(const SubItemIterator& rhs) const {
                return m_start == rhs.m_start;
            }

            bool operator!=(const SubItemIterator& rhs) const {
                return m_start != rhs.m_start;
            }

            const TypedItem& operator*() {
                return *reinterpret_cast<const TypedItem*>(m_start);
            }
            
            const TypedItem* operator->() {
                return reinterpret_cast<const TypedItem*>(m_start);
            }

        protected:

            const char*       m_start;
            const char* const m_end;

        }; // class SubItemIterator

        // serialized form of OSM object
        class Object : public TypedItem {

        public:

            osm_object_id_t id() const {
                return m_id;
            }

            Object& id(osm_object_id_t id) {
                m_id = id;
                return *this;
            }

            Object& id(const char* id) {
                return this->id(Osmium::string_to_osm_object_id_t(id));
            }

            osm_version_t version() const {
                return m_visible_and_version & 0x7fffffff;
            }

            bool visible() const {
                return m_visible_and_version & 0x80000000;
            }

            Object& version(osm_version_t version) {
                m_visible_and_version = (m_visible_and_version & 0x80000000) | (version & 0x7fffffff);
                return *this;
            }

            Object& version(const char* version) {
                return this->version(Osmium::string_to_osm_version_t(version));
            }

            Object& visible(bool visible) {
                if (visible) {
                    m_visible_and_version |= 0x80000000;
                } else {
                    m_visible_and_version &= 0x7fffffff;
                }
                return *this;
            }

            osm_changeset_id_t changeset() const {
                return m_changeset;
            }

            Object& changeset(osm_changeset_id_t changeset) {
                m_changeset = changeset;
                return *this;
            }

            Object& changeset(const char* changeset) {
                return this->changeset(Osmium::string_to_osm_changeset_id_t(changeset));
            }

            osm_user_id_t uid() const {
                return m_uid;
            }

            Object& uid(osm_user_id_t uid) {
                m_uid = uid;
                return *this;
            }

            Object& uid(const char* uid) {
                return this->uid(Osmium::string_to_osm_user_id_t(uid));
            }

            bool user_is_anonymous() const {
                return m_uid == -1;
            }

            time_t timestamp() const {
                return m_timestamp;
            }

            /**
             * Set the timestamp when this object last changed.
             * @param timestamp Time in seconds since epoch.
             * @return Reference to object to make calls chainable.
             */
            Object& timestamp(time_t timestamp) {
                m_timestamp = timestamp;
                return *this;
            }

            const char* user_position() const {
                return self() + sizeof(Object) + (type().is_node() ? sizeof(Osmium::OSM::Position) : 0);
            }

            const char* user() const {
                return user_position() + sizeof(size_t);
            }

            size_t user_length() const {
                return *reinterpret_cast<const size_t*>(user_position());
            }

            typedef SubItemIterator iterator;

            const char* subitems_position() const {
                return user_position() + sizeof(size_t) + padded_length(user_length());
            }

            iterator begin() const {
                return iterator(subitems_position(), self() + padded_size());
            }

            iterator end() const {
                return iterator(self() + padded_size(), self() + padded_size());
            }

        private:

            int64_t m_id;
            uint32_t m_visible_and_version;
            uint32_t m_timestamp;
            uint32_t m_uid;
            uint32_t m_changeset;

        };

        // serialized form of OSM node
        class Node : public Object {

        public:

            const Osmium::OSM::Position position() const {
                return m_position;
            }

            Node& position(const Osmium::OSM::Position& position) {
                m_position = position;
                return *this;
            }

        private:

            Osmium::OSM::Position m_position;

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

        template <class TMember>
        class Collection : public TypedItem {

        public:

            typedef CollectionIterator<TMember> iterator;

            iterator begin() const {
                return iterator(self() + sizeof(Collection<TMember>), self() + size());
            }

            iterator end() const {
                return iterator(self() + size(), self() + size());
            }

        }; // class Collection

        class Tag : public Item {

        public:

            const char* key() const {
                return self();
            }

            const char* value() const {
                return self() + strlen(self()) + 1;
            }

            const char* next() const {
                const char* data = value();
                return data + strlen(data) + 1;
            }

        }; // class Tag

        typedef Collection<Tag> TagList;

        template <>
        struct item_traits<TagList> {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_tag_list;
        };

        class WayNode : public Item {

        public:

            WayNode(osm_object_id_t id) :
                m_id(id) {
            }

            osm_object_id_t id() const {
                return m_id;
            }

            const char* next() const {
                return reinterpret_cast<const char*>(this + 1);
            }

        private:

            osm_object_id_t m_id;

        }; // class WayNode

        class WayNodeWithPosition : public WayNode {

        public:

            WayNodeWithPosition(osm_object_id_t id, const Osmium::OSM::Position& position) :
                WayNode(id),
                m_position(position) {
            }

            Osmium::OSM::Position position() const {
                return m_position;
            }

            const char* next() const {
                return reinterpret_cast<const char*>(this + 1);
            }

        private:

            Osmium::OSM::Position m_position;

        }; // class WayNodeWithPosition

        typedef Collection<WayNode> WayNodeList;
        typedef Collection<WayNodeWithPosition> WayNodeWithPositionList;

        template <>
        struct item_traits<WayNodeList> {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_way_node_list;
        };

        template <>
        struct item_traits<WayNodeWithPositionList> {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_way_node_with_position_list;
        };

        class RelationMember : public Item {

        public:

            RelationMember(osm_object_id_t ref=0, ItemType type=ItemType(), bool full=false) :
                m_ref(ref),
                m_type(type),
                m_flags(full ? 1 : 0) {
            }

            osm_object_id_t ref() const {
                return m_ref;
            }

            ItemType type() const {
                return m_type;
            }

            bool full_member() const {
                return m_flags == 1;
            }

            const char* role_position() const {
                return self() + sizeof(RelationMember);
            }

            const char* role() const {
                return role_position() + sizeof(size_t);
            }

            const char* end() const {
                const char* current = reinterpret_cast<const char*>(this + 1);
                return current + sizeof(size_t) + padded_length(*reinterpret_cast<const size_t*>(current));
            }

            const char* next() const {
                if (full_member()) {
                    return end() + reinterpret_cast<const Osmium::Ser::TypedItem*>(end())->size();
                } else {
                    return end();
                }
            }

            const Osmium::Ser::Object& get_object() const {
                return *reinterpret_cast<const Osmium::Ser::Object*>(end());
            }

        private:

            osm_object_id_t m_ref;
            ItemType m_type;
            uint32_t m_flags;

        }; // class RelationMember

        typedef Collection<RelationMember> RelationMemberList;

        template <>
        struct item_traits<RelationMemberList> {
            static const uint32_t itemtype = Osmium::Ser::ItemType::itemtype_relation_member_list;
        };

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_ITEM_HPP
