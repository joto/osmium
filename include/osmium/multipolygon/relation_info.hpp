#ifndef OSMIUM_MULTIPOLYGON_RELATION_INFO_HPP
#define OSMIUM_MULTIPOLYGON_RELATION_INFO_HPP

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

namespace Osmium {

    namespace MultiPolygon {

        /**
         * Information about a Relation needed for MultiPolygon assembly.
         */
        class RelationInfo : public Osmium::Relations::RelationInfo {

            bool m_is_boundary;

        public:

            RelationInfo() :
                Osmium::Relations::RelationInfo(),
                m_is_boundary(false) {
            }

            RelationInfo(const shared_ptr<Osmium::OSM::Relation const>& relation, bool is_boundary) :
                Osmium::Relations::RelationInfo(relation),
                m_is_boundary(is_boundary) {
            }

            bool is_boundary() const {
                return m_is_boundary;
            }

        }; // class RelationInfo

    } // namespace MultiPolygon

} // namespace Osmium

#endif // OSMIUM_MULTIPOLYGON_RELATION_INFO_HPP
