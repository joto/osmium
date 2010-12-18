#ifndef OSMIUM_OSM_MULTIPOLYGON_HPP
#define OSMIUM_OSM_MULTIPOLYGON_HPP

#include <sys/types.h>
#include <sstream>
#include <map>

#ifdef WITH_GEOS
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKBReader.h>
#endif

#ifdef WITH_SHPLIB
#include <shapefil.h>
#endif

#include "OsmWay.hpp"
#include "timer.h"

#define OUTPUT_SRID 4326

/*
struct Tag 
{
    string key;
    string value;
    Tag(const char *a, const char *b) { key.assign(a); value.assign(b); }
};

struct Member
{
    int id;
    innerouter_t innerouter;
};
*/

namespace Osmium {

    namespace OSM {

        enum innerouter_t { UNSET, INNER, OUTER };
        enum direction_t { NO_DIRECTION, CLOCKWISE, COUNTERCLOCKWISE };

        class WayInfo {

          public:

            Osmium::OSM::Way *way;
            int used;
            int sequence;
            bool invert;
            int poly_id; 
            bool duplicate; 
            std::string errorhint;
            innerouter_t innerouter;
            innerouter_t orig_innerouter;

            WayInfo() {
                way = NULL;
                used = -1;
                poly_id=-1;
                innerouter = UNSET;
                orig_innerouter = UNSET;
                sequence = 0;
                invert = false;
                duplicate = false;
            }

            WayInfo(Osmium::OSM::Way *w, innerouter_t io) {
                way = w;
                orig_innerouter = io;
                used = -1;
                poly_id= -1;
                innerouter = UNSET;
                sequence = 0;
                invert = false;
                duplicate = false;
            }
        };

        class RingInfo {

          public:

            geos::geom::Polygon *polygon;
            direction_t direction;
            std::vector<WayInfo *> ways;
            std::vector<RingInfo *> inner_rings;
            bool nested;
            RingInfo *contained_by;
            int ring_id;

            RingInfo() {
                direction = NO_DIRECTION;
                contained_by = NULL;
                polygon = NULL;
                nested = false;
                ring_id = -1;
            }
        };

        /// virtual parent class for MultipolygonFromWay and MultipolygonFromRelation
        class Multipolygon : public Object {

        }; // class Multipolygon

        /***
        * multipolygon created from a way (so strictly speaking this will
        * always be a simple polygon)
        */
        class MultipolygonFromWay : public Multipolygon {

            public:

            /// the way this multipolygon was build from
            Way *way;

            MultipolygonFromWay(Way *w) : way(w) {
                num_tags = w->tag_count();
                tags = w->tags;
            }

            osm_object_type_t get_type() const {
                return MULTIPOLYGON_FROM_WAY;
            }

            osm_object_id_t get_id() const {
                return way->get_id();
            }

#ifdef WITH_GEOS
            bool build_geometry() {
                geometry = way->get_geometry();
                return true;
            }
#endif

#ifdef WITH_SHPLIB
            SHPObject *create_shpobject(int shp_type) {
                if (shp_type != SHPT_POLYGON) {
                    throw std::runtime_error("a multipolygon can only be added to a shapefile of type polygon");
                }
                return SHPCreateSimpleObject(shp_type, way->node_count(), way->lon, way->lat, NULL);
            }
#endif

        }; // class MultipolygonFromWay

        /***
        * multipolygon created from a relation with tag type=multipolygon or type=boundary
        */
        class MultipolygonFromRelation : public Multipolygon {

            bool boundary; ///< was this multipolygon created from relation with tag type=boundary?

          public:

            /// the relation this multipolygon was build from
            Relation *relation;

            /// the member ways of this multipolygon
            std::vector<Osmium::OSM::Way> member_ways;

            /// number of ways in this multipolygon
            int num_ways;

            /// how many ways are missing before we can build this multipolygon
            int missing_ways;

            std::string geometry_error_message;

          private:

            timer write_complex_poly_timer;
            timer assemble_ways_timer;
            timer assemble_nodes_timer;
            timer make_one_ring_timer;
            timer mor_polygonizer_timer;
            timer mor_union_timer;
            timer contains_timer;
            timer extra_polygons_timer;
            timer polygon_build_timer;
            timer inner_ring_touch_timer;
            timer polygon_intersection_timer;
            timer multipolygon_build_timer;
            timer multipolygon_write_timer;
            timer error_write_timer;

          public:

            MultipolygonFromRelation(Relation *r, bool b) : boundary(b), relation(r) {
                num_ways = 0;
                init();
            }

            ~MultipolygonFromRelation() {
                delete relation;
            }

            osm_object_type_t get_type() const {
                return MULTIPOLYGON_FROM_RELATION;
            }

            osm_object_id_t get_id() const {
                return relation->get_id();
            }

#ifdef WITH_GEOS
            bool build_geometry(Relation *r);

            bool build_geometry() {
                std::cerr << "going to build multipolygon geometry\n";
                if (!build_geometry(relation)) {
                    std::cerr << "building mp failed: " << geometry_error_message << "\n";
                }
                return true;
            }
#endif

          private:

            RingInfo *make_one_ring(std::vector<WayInfo> &ways, osm_object_id_t first, osm_object_id_t last, int ringcount, int sequence);
            bool geometry_error(const char *message);

            void init() {
                geometry = NULL;
                // delete pm;

#ifdef PROFILING
                timers.push_back(pair<string, timer *> ("   thereof assemble_ways", &assemble_ways_timer));
                timers.push_back(pair<string, timer *> ("   thereof make_one_ring", &make_one_ring_timer));
                timers.push_back(pair<string, timer *> ("      thereof union", &mor_union_timer));
                timers.push_back(pair<string, timer *> ("      thereof polygonizer", &mor_polygonizer_timer));
                timers.push_back(pair<string, timer *> ("   thereof contains", &contains_timer));
                timers.push_back(pair<string, timer *> ("   thereof extra_polygons", &extra_polygons_timer));
                timers.push_back(pair<string, timer *> ("   thereof polygon_build", &polygon_build_timer));
                timers.push_back(pair<string, timer *> ("      thereof inner_ring_touch", &inner_ring_touch_timer));
                timers.push_back(pair<string, timer *> ("      thereof intersections", &polygon_intersection_timer));
                timers.push_back(pair<string, timer *> ("   thereof multipolygon_build", &multipolygon_build_timer));
                timers.push_back(pair<string, timer *> ("   thereof multipolygon_write", &multipolygon_write_timer));
                timers.push_back(pair<string, timer *> ("   thereof error_write", &error_write_timer));
#endif
            }

            public:

#ifdef WITH_SHPLIB
#ifdef WITH_GEOS
            bool dumpGeometry(const geos::geom::Geometry *g, std::vector<int>& partStart, std::vector<double>& x, std::vector<double>& y)
            {
                switch(g->getGeometryTypeId())
                {
                    case geos::geom::GEOS_MULTIPOLYGON:
                    case geos::geom::GEOS_MULTILINESTRING:
                    {
                        for (size_t i=0; i<g->getNumGeometries(); i++)
                        {
                            if (!dumpGeometry(g->getGeometryN(i), partStart, x, y)) return false;
                        }
                        break;
                    }
                    case geos::geom::GEOS_POLYGON:
                    {
                        geos::geom::Polygon *p = (geos::geom::Polygon *) g;
                        if (!dumpGeometry(p->getExteriorRing(), partStart, x, y)) return false;
                        for (size_t i=0; i<p->getNumInteriorRing(); i++)
                        {
                            if (!dumpGeometry(p->getInteriorRingN(i), partStart, x, y)) return false; 
                        }
                        break;
                    }
                    case geos::geom::GEOS_LINESTRING:
                    case geos::geom::GEOS_LINEARRING:
                    {
                        partStart.push_back(x.size());
                        const geos::geom::CoordinateSequence *cs = ((geos::geom::LineString *) g)->getCoordinatesRO();
                        for (size_t i = 0; i < cs->getSize(); i++)
                        {
                            x.push_back(cs->getX(i));
                            y.push_back(cs->getY(i));
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error("invalid geometry type encountered");
                }
                return true;
            }

            SHPObject *create_shpobject(int shp_type) {
                if (shp_type != SHPT_POLYGON) {
                    throw std::runtime_error("a multipolygon can only be added to a shapefile of type polygon");
                }

                const geos::geom::Geometry *g = get_geometry();

                if (!g) {
                    return NULL;
                }

                std::vector<double> x;
                std::vector<double> y;
                std::vector<int> partStart;

                dumpGeometry(g, partStart, x, y);

                int *ps = new int[partStart.size()];
                for (size_t i=0; i<partStart.size(); i++) ps[i]=partStart[i];
                double *xx = new double[x.size()];
                for (size_t i=0; i<x.size(); i++) xx[i]=x[i];
                double *yy = new double[y.size()];
                for (size_t i=0; i<y.size(); i++) yy[i]=y[i];

                SHPObject *o = SHPCreateObject(
                    SHPT_POLYGON,       // type
                    -1,                 // id
                    partStart.size(),   // nParts
                    ps,                 // panPartStart
                    NULL,               // panPartType
                    x.size(),           // nVertices,
                    xx, 
                    yy,
                    NULL,
                    NULL);
                //cout << "rewind: " << SHPRewindObject(h, o) << endl;
                delete[] ps;
                delete[] xx;
                delete[] yy;
                
                return o;
            }
#endif
#endif

        }; // class MultipolygonFromRelation

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_MULTIPOLYGON_HPP
