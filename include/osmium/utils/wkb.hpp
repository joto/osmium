#ifndef OSMIUM_WKB_HPP
#define OSMIUM_WKB_HPP

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

// see 99-049_OpenGIS_Simple_Features_Specification_For_SQL_Rev_1.1.pdf for WKB
// see PostGIS doc/ZMSgeoms.txt for EWKB

enum wkbGeometryType {
    wkbPoint               = 1,
    wkbLineString          = 2,
    wkbPolygon             = 3,
    wkbMultiPoint          = 4,
    wkbMultiLineString     = 5,
    wkbMultiPolygon        = 6,
    wkbGeometryCollection  = 7,

    // SRID-presence flag (EWKB)
    wkbSRID                = 0x20000000,

    // | 0x20000000 (SRID)
    wkbPointS              = 0x20000001,
    wkbLineStringS         = 0x20000002,
    wkbPolygonS            = 0x20000003,
    wkbMultiPointS         = 0x20000004,
    wkbMultiLineStringS    = 0x20000005,
    wkbMultiPolygonS       = 0x20000006,
    wkbGeometryCollectionS = 0x20000007
};

enum wkbByteOrder {
    wkbXDR = 0,         // Big Endian
    wkbNDR = 1          // Little Endian
};

struct Point {
    double x;
    double y;
};

class WKBPoint {
    uint8_t       padding[3];

public:

    uint8_t       byteOrder;
    uint32_t      wkbType; // wkbPoint = 1
    Point         point;

    WKBPoint() : byteOrder(wkbNDR), wkbType(wkbPoint) {
    }

    const char *to_hex() const {
        static const char *lookup_hex = "0123456789abcdef";
        static char buffer[2*(sizeof(uint8_t) + sizeof(uint32_t) + sizeof(Point))+1];

        for (unsigned int i = 0; i < sizeof(byteOrder) + sizeof(wkbType) + sizeof(point); i++) {
            buffer[2*i]   = lookup_hex[((const unsigned char *)&byteOrder)[i] >> 4];
            buffer[2*i+1] = lookup_hex[((const unsigned char *)&byteOrder)[i] & 0xf];
        }

        buffer[sizeof(buffer)-1] = 0;
        return buffer;
    }

};

struct WKBPointS {
    uint8_t       byteOrder;
    uint32_t      wkbType; // wkbPoint = 1
    uint32_t      SRID;
    Point         point;
};

struct WKBLineString {
    uint8_t       byteOrder;
    uint32_t      wkbType; // wkbLinestring = 2
    uint32_t      numPoints;
    Point         points[1];
};

struct LinearRing  {
    uint32_t      numPoints;
    Point         points[1];
};

struct WKBPolygon {
    uint8_t       byteOrder;
    uint32_t      wkbType; // wkbPolygon = 3
    uint32_t      numRings;
    LinearRing    rings[1];
};

struct WKBMultiPoint {
    uint8_t       byteOrder;
    uint32_t      wkbType; // wkbMultiPoint = 4
    uint32_t      num_wkbPoints;
    WKBPoint      wkbPoints[1];
};

struct WKBMultiLineString {
    uint8_t       byteOrder;
    uint32_t      wkbType; // wkbMultiLineString = 5
    uint32_t      num_wkbLineStrings;
    WKBLineString wkbLineStrings[1];
};

struct WKBMultiPolygon {
    uint8_t       byteOrder;
    uint32_t      wkbType; // wkbMultiPolygon = 6
    uint32_t      num_wkbPolygons;
    WKBPolygon    wkbPolygons[1];
};

#if 0
struct WKBGeometry {
    union {
        WKBPoint              point;
        WKBLineString         linestring;
        WKBPolygon            polygon;
//        WKBGeometryCollection collection;
        WKBMultiPoint         mpoint;
        WKBMultiLineString    mlinestring;
        WKBMultiPolygon       mpolygon;
    };
};

struct WKBGeometryCollection {
    uint8_t       byte_order;
    uint32_t      wkbType; // wkbGeometryCollection = 7
    uint32_t      num_wkbGeometries;
    WKBGeometry   wkbGeometries[];
};
#endif

#endif // OSMIUM_WKB_HPP
