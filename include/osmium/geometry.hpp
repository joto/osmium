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

#include <sstream>

#ifdef OSMIUM_WITH_GEOS
# include <geos/geom/GeometryFactory.h>
# include <geos/geom/PrecisionModel.h>
#endif // OSMIUM_WITH_GEOS

#include <osmium/exceptions.hpp>

namespace Osmium {

    namespace Geometry {

        /// %OSM data always uses SRID 4326 (WGS84).
        const int srid = 4326;

        /**
         * Type of WKB geometry.
         * These definitions are from
         * 99-049_OpenGIS_Simple_Features_Specification_For_SQL_Rev_1.1.pdf (for WKB)
         * and http://trac.osgeo.org/postgis/browser/trunk/doc/ZMSgeoms.txt (for EWKB).
         * They are used to encode geometries into the WKB format.
         */
        enum wkbGeometryType {
            wkbPoint               = 1,
            wkbLineString          = 2,
            wkbPolygon             = 3,
            wkbMultiPoint          = 4,
            wkbMultiLineString     = 5,
            wkbMultiPolygon        = 6,
            wkbGeometryCollection  = 7,

            // SRID-presence flag (EWKB)
            wkbSRID                = 0x20000000
        };

        /**
         * Byte order marker in WKB geometry.
         */
        enum wkbByteOrder {
            wkbXDR = 0,         // Big Endian
            wkbNDR = 1          // Little Endian
        };

#ifdef OSMIUM_WITH_GEOS
        /**
         * Return pointer to a static GEOS GeometryFactory object created the
         * first time this function is run. This is used by all functions in
         * Osmium that need to create GEOS geometries.
         */
        geos::geom::GeometryFactory* geos_geometry_factory() {
            static geos::geom::PrecisionModel pm;
            static geos::geom::GeometryFactory factory(&pm, -1);
            return &factory;
        }
#endif // OSMIUM_WITH_GEOS

        class Geometry;

        /**
         * This helper class is used to allow writing geometries in different
         * formats to an output stream.
         *
         * If we'd just write
         * @code
         *  Osmium::Geometry::Geometry geometry;
         *  std::stream out << geometry;
         * @endcode
         * we would not know in which format to write.
         *
         * Instead we can write
         * @code
         *   std::stream out << geometry.as_WKT();
         * @endcode
         * and this class magically makes this work.
         *
         * @see Geometry::AsWKT
         * @see Geometry::AsWKB
         * @see Geometry::AsHexWKB
         */
        template <typename T>
        struct StreamFormat {
            StreamFormat(const Geometry& geometry, bool with_srid) : m_geometry(geometry), m_with_srid(with_srid) {}
            const Geometry& m_geometry;
            const bool m_with_srid;
        };

        /**
         * Output operator for StreamFormat.
         */
        template <typename T>
        std::ostream& operator<<(std::ostream& out, StreamFormat<T> format) {
            return format.m_geometry.write_to_stream(out, format, format.m_with_srid);
        }

        /**
         * Abstract base class for all Osmium geometry classes. Geometries of different
         * types are created from OSM objects (nodes, ways, relations). Geometries
         * can be written out and transformed in different ways.
         */
        class Geometry {

        public:

            virtual ~Geometry() {
            }

            // These types are never instantiated, they are used in the write_to_stream()
            // methods below as parameters to make the overloading mechanism choose the
            // right version.
            typedef StreamFormat<struct WKT_>  AsWKT;
            typedef StreamFormat<struct WKB_>  AsWKB;
            typedef StreamFormat<struct HWKB_> AsHexWKB;

            AsWKT as_WKT(bool with_srid=false) const {
                return AsWKT(*this, with_srid);
            }

            AsWKB as_WKB(bool with_srid=false) const {
                return AsWKB(*this, with_srid);
            }

            AsHexWKB as_HexWKB(bool with_srid=false) const {
                return AsHexWKB(*this, with_srid);
            }

            /**
             * Write a value as binary to an output stream.
             *
             * @tparam T Type of value.
             */
            template<typename T>
            inline void write_binary(std::ostream& out, const T value) const {
                out.write(reinterpret_cast<const char *>(&value), sizeof(T));
            }

            /**
             * Write a value as hex encoding of binary to an output stream.
             *
             * @tparam T Type of value.
             */
            template<typename T>
            inline void write_hex(std::ostream& out, const T value) const {
                static const char *lookup_hex = "0123456789ABCDEF";
                for (const char* in = reinterpret_cast<const char*>(&value); in < reinterpret_cast<const char*>(&value) + sizeof(T); ++in) {
                    out << lookup_hex[(*in >> 4) & 0xf]
                        << lookup_hex[*in & 0xf];
                }
            }

            /**
             * Write header of WKB data structure as binary to output stream.
             * The header contains:
             * - the byte order marker
             * - the geometry type
             * - (optionally) the SRID
             */
            inline void write_binary_wkb_header(std::ostream& out, bool with_srid, uint32_t type) const {
                write_binary<uint8_t>(out, wkbNDR);
                if (with_srid) {
                    write_binary<uint32_t>(out, type | wkbSRID);
                    write_binary<uint32_t>(out, srid);
                } else {
                    write_binary<uint32_t>(out, type);
                }
            }

            /**
             * Write header of WKB data structure as hex encoding of binary to output stream.
             * The header contains:
             * - the byte order marker
             * - the geometry type
             * - (optionally) the SRID
             */
            inline void write_hex_wkb_header(std::ostream& out, bool with_srid, uint32_t type) const {
                write_hex<uint8_t>(out, wkbNDR);
                if (with_srid) {
                    write_hex<uint32_t>(out, type | wkbSRID);
                    write_hex<uint32_t>(out, srid);
                } else {
                    write_hex<uint32_t>(out, type);
                }
            }

            /// Write geometry as WKT to output stream.
            virtual std::ostream& write_to_stream(std::ostream& out, AsWKT,    bool with_srid=false) const = 0;

            /// Write geometry as WKB to output stream.
            virtual std::ostream& write_to_stream(std::ostream& out, AsWKB,    bool with_srid=false) const = 0;

            /// Write geometry as hex encoded WKB to output stream.
            virtual std::ostream& write_to_stream(std::ostream& out, AsHexWKB, bool with_srid=false) const = 0;

#ifdef  OSMIUM_WITH_SHPLIB
            virtual SHPObject *create_shp_object() const {
                return NULL;
            }
#endif // OSMIUM_WITH_SHPLIB

#ifdef OSMIUM_WITH_JAVASCRIPT
            v8::Handle<v8::Value> js_to_wkt(const v8::Arguments& args) {
                std::ostringstream oss;
                bool with_srid = false;
                if (args.Length() >= 1) {
                    with_srid = args[0]->ToBoolean()->Value();
                }
                oss << this->as_WKT(with_srid);
                return v8::String::New(oss.str().c_str());
            }

            v8::Handle<v8::Value> js_to_wkb(const v8::Arguments& args) {
                std::ostringstream oss;
                bool with_srid = false;
                if (args.Length() >= 1) {
                    with_srid = args[0]->ToBoolean()->Value();
                }
                oss << this->as_WKB(with_srid);
                return v8::String::New(oss.str().c_str());
            }

            v8::Handle<v8::Value> js_to_hexwkb(const v8::Arguments& args) {
                std::ostringstream oss;
                bool with_srid = false;
                if (args.Length() >= 1) {
                    with_srid = args[0]->ToBoolean()->Value();
                }
                oss << this->as_HexWKB(with_srid);
                return v8::String::New(oss.str().c_str());
            }

            struct JavascriptTemplate : public Osmium::Javascript::Template {

                JavascriptTemplate() : Osmium::Javascript::Template() {
                    js_template->Set("toWKT",    v8::FunctionTemplate::New(function_template<Geometry, &Geometry::js_to_wkt>));
                    js_template->Set("toWKB",    v8::FunctionTemplate::New(function_template<Geometry, &Geometry::js_to_wkb>));
                    js_template->Set("toHexWKB", v8::FunctionTemplate::New(function_template<Geometry, &Geometry::js_to_hexwkb>));
                }

            };
#endif // OSMIUM_WITH_JAVASCRIPT

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
