#ifndef OSMIUM_GEOMETRY_HPP
#define OSMIUM_GEOMETRY_HPP

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

#include <osmium/exceptions.hpp>

namespace Osmium {

    namespace Geometry {

        class Geometry;

        template <typename T>
        struct StreamFormat {
            StreamFormat(const Geometry& t) : m_t(t) {}
            const Geometry& m_t;
        };

        template <typename T>
        std::ostream& operator <<(std::ostream& out, StreamFormat<T> format) {
            return format.m_t.write_to_stream(out, format);
        }

        typedef StreamFormat<struct WKT_>   AsWKT;
        typedef StreamFormat<struct EWKT_>  AsEWKT;
        typedef StreamFormat<struct WKB_>   AsWKB;
        typedef StreamFormat<struct EWKB_>  AsEWKB;
        typedef StreamFormat<struct HWKB_>  AsHexWKB;
        typedef StreamFormat<struct HEWKB_> AsHexEWKB;

        class Geometry {

        public:

            virtual ~Geometry() {
            }

            AsWKT as_WKT() const {
                return AsWKT(*this);
            }

            AsEWKT as_EWKT() const {
                return AsEWKT(*this);
            }

            AsHexWKB as_HexWKB() const {
                return AsHexWKB(*this);
            }

            virtual std::ostream& write_to_stream(std::ostream& out, AsWKT) const = 0;

            virtual std::ostream& write_to_stream(std::ostream& out, AsEWKT) const {
                return out << "SRID=4326;" << this->as_WKT();
            }

            virtual std::ostream& write_to_stream(std::ostream& out, AsHexWKB) const = 0;

#ifdef  OSMIUM_WITH_SHPLIB
            virtual SHPObject *create_shp_object() const {
                return NULL;
            }
#endif // OSMIUM_WITH_SHPLIB

        }; // class Geometry

        /**
         * This helper class is used for writing out lists of coordinates
         * to an output stream. It is intended to be used as a functor argument
         * in a for_each() call iterating over something that holds a list of
         * TLonLat objects.
         *
         * @tparam TLonLat A class that has the methods 'double lon();' and 'double lat();'
         */
        template <typename TLonLat>
        class LonLatListWriter {

        public:

            LonLatListWriter(std::ostream& out,      ///< The output stream
                             char delim_lonlat=' ',  ///< The delimiter between longitude and latitude
                             char delim_items=',')   ///< The delimiter between consecutive coordinates
                           : m_out(out),
                             m_delim_lonlat(delim_lonlat),
                             m_delim_items(delim_items),
                             m_first(true) {
            }

            void operator()(const TLonLat& lonlat) {
                if (m_first) {
                    m_first = false;
                } else {
                    m_out << m_delim_items;
                }
                m_out << lonlat.lon() << m_delim_lonlat << lonlat.lat();
            }

        private:

            std::ostream& m_out;
            char m_delim_lonlat;
            char m_delim_items;
            bool m_first;

        }; // class LonLatListWriter

    } // namespace Geometry

} // namespace Osmium

#endif // OSMIUM_GEOMETRY_HPP
