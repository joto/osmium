#ifndef OSMIUM_GEOMBUILDER_HPP
#define OSMIUM_GEOMBUILDER_HPP

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

#include <string.h>
#include <geos/geom/MultiPolygon.h>
#include <osmium/storage/byid.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/geometry/point.hpp>


typedef Osmium::Storage::Mmap<Osmium::OSM::Position> storage_mmap_t;
typedef Osmium::Handler::CoordinatesForWays<storage_mmap_t, storage_mmap_t> cfw_handler_t;

namespace Osmium {

    class OsmGeometryReader : public Osmium::Handler::Base {
        std::vector<geos::geom::Geometry*> outer;
        storage_mmap_t store_pos;
        storage_mmap_t store_neg;
        cfw_handler_t* handler_cfw;

    public:
        OsmGeometryReader() : Base() {
            handler_cfw = new cfw_handler_t(store_pos, store_neg);
        }

        virtual ~OsmGeometryReader() {
            delete handler_cfw;

            geos::geom::GeometryFactory *f = Osmium::Geometry::geos_geometry_factory();
            for (uint32_t i=0; i < outer.size(); i++) {
                f->destroyGeometry(outer[i]);
            }
            outer.clear();
        }

        void init(Osmium::OSM::Meta& meta) {
            handler_cfw->init(meta);
        }

        void node(Osmium::OSM::Node *node) {
            handler_cfw->node(node);
        }

        void way(Osmium::OSM::Way *way) {
            handler_cfw->way(way);

            if (!way->is_closed()) {
                std::cerr << "open way " << way->id() << " in osm-input" << std::endl;
                return;
            }

            geos::geom::Geometry *geom = way->create_geos_polygon_geometry();
            outer.push_back(geom);
        }

        void after_nodes() {
            handler_cfw->after_nodes();
        }

        void final() {
            handler_cfw->final();
        }

        geos::geom::Geometry *buildGeom() const {
            // shorthand to the geometry factory
            geos::geom::GeometryFactory *f = Osmium::Geometry::geos_geometry_factory();
            geos::geom::MultiPolygon *outerPoly = f->createMultiPolygon(outer);
            return outerPoly;
        }
    };

    class GeometryReader {

        /// maximum length of a line in a .poly file
        static const int polyfile_linelen = 2048;

    public:

        /**
         * read a .poly file and generate a geos Geometry from it.
         *
         * .poly-files are simple:
         *   - a title
         *   - 1..n polygons
         *     - a polygon number, possibly with a ! in front so signalize holes
         *     - 1..n lines with floating point latitudes and longitudes
         *     - END token
         *   - END token
         *
         * usually the Geometry read from .poly files are used together with an
         * geos::algorithm::locate::IndexedPointInAreaLocator to check for nodes
         * being located inside the polygon.
         *
         * this method returns NULL if the .poly file can't be read.
         */
        static geos::geom::Geometry *fromPolyFile(const std::string &file) {
            // shorthand to the geometry factory
            geos::geom::GeometryFactory *f = Osmium::Geometry::geos_geometry_factory();

            // pointer to coordinate vector
            std::vector<geos::geom::Coordinate> *c = NULL;

            // vectors of outer and inner polygons
            std::vector<geos::geom::Geometry*> *outer = new std::vector<geos::geom::Geometry*>();
            std::vector<geos::geom::Geometry*> *inner = new std::vector<geos::geom::Geometry*>();

            // file pointer to .poly file
            FILE *fp = fopen(file.c_str(), "r");
            if(!fp) {
                std::cerr << "unable to open polygon file " << file << std::endl;
                return NULL;
            }

            // line buffer
            char line[polyfile_linelen];

            // read title line
            if(!fgets(line, polyfile_linelen-1, fp)) {
                std::cerr << "unable to read title line from polygon file " << file << std::endl;
                return NULL;
            }
            line[polyfile_linelen-1] = '\0';

            // is this polygon an inner polygon
            bool isinner = false;

            // are we currently inside parsing one polygon
            bool ispoly = false;

            // double x / y coords
            double x = 0, y = 0;

            // read through the file
            while(!feof(fp)) {
                // read a line
                if(!fgets(line, polyfile_linelen-1, fp)) {
                    std::cerr << "unable to read line from polygon file " << file << std::endl;
                    return NULL;
                }
                line[polyfile_linelen-1] = '\0';

                // when we're currently outside a polygon
                if(!ispoly) {
                    // if this is an end-line
                    if(0 == strncmp(line, "END", 3)) {
                        // cancel parsing
                        break;
                    }

                    // this is considered a polygon-start line
                    // if it begins with ! it signales the start of an inner polygon
                    isinner = (line[0] == '!');

                    // remember we're inside a polygon
                    ispoly = true;

                    // create a new coordinate sequence
                    c = new std::vector<geos::geom::Coordinate>();

                // when we're currently inside a polygon
                } else {
                    // if this is an end-line
                    if(0 == strncmp(line, "END", 3)) {
                        // build a polygon from the coordniate vector
                        geos::geom::Geometry* poly = f->createPolygon(
                            f->createLinearRing(
                                f->getCoordinateSequenceFactory()->create(c)
                            ),
                            NULL
                        );

                        // add it to the appropriate polygon vector
                        if(isinner) {
                            inner->push_back(poly);
                        } else {
                            outer->push_back(poly);
                        }

                        // remember we're now outside a polygon
                        ispoly = false;

                    // an ordinary line
                    } else {
                        // try to parse it using sscanf
                        if(2 != sscanf(line, " %lE %lE", &x, &y)) {
                            std::cerr << "unable to parse line from polygon file " << file << ": " << line;
                            return NULL;
                        }

                        // push the parsed coordinate into the coordinate vector
                        c->push_back(geos::geom::Coordinate(x, y, DoubleNotANumber));
                    }
                }
            }

            // check that the file ended with END
            if(0 != strncmp(line, "END", 3)) {
                std::cerr << "polygon file " << file << " does not end with END token" << std::endl;
                return NULL;
            }

            // close the file pointer
            fclose(fp);

            // build MultiPolygons from the vectors of outer and inner polygons
            geos::geom::MultiPolygon *outerPoly = f->createMultiPolygon(outer);
            geos::geom::MultiPolygon *innerPoly = f->createMultiPolygon(inner);

            // generate a MultiPolygon containing the difference of those two
            geos::geom::Geometry *poly = outerPoly->difference(innerPoly);

            // destroy the both MultiPolygons
            f->destroyGeometry(outerPoly);
            f->destroyGeometry(innerPoly);

            // and return their difference
            return poly;
        } // fromPolyFile

        static geos::geom::Geometry *fromOsmFile(const std::string &file) {
            Osmium::OSMFile infile(file);
            Osmium::OsmGeometryReader reader;
            infile.read(reader);
            geos::geom::Geometry *geom = reader.buildGeom();

            return geom;
        }

        /**
         * construct a geos Geometry from a BoundingBox string.
         *
         * this method returns NULL if the string can't be read.
         */
        static geos::geom::Geometry *fromBBox(const std::string &bbox) {
            double minlon, minlat, maxlon, maxlat;
            if(4 != sscanf(bbox.c_str(), "%lf,%lf,%lf,%lf", &minlon, &minlat, &maxlon, &maxlat)) {
                std::cerr << "invalid BBox string: " << bbox << std::endl;
                return NULL;
            }

            // build the Geometry from the coordinates
            return fromBBox(minlon, minlat, maxlon, maxlat);
        }

        static geos::geom::Geometry *fromBBox(double minlon, double minlat, double maxlon, double maxlat) {
            // create an Envelope and convert it to a polygon
            geos::geom::Envelope *e = new geos::geom::Envelope(minlon, maxlon, minlat, maxlat);
            geos::geom::Geometry *p = Osmium::Geometry::geos_geometry_factory()->toGeometry(e);

            delete e;
            return p;
        }

    }; // class GeomBuilder

} // namespace Osmium

#endif // OSMIUM_GEOMBUILDER_HPP
