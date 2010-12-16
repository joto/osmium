#ifndef OSMIUM_OSM_MULTIPOLYGON_HPP
#define OSMIUM_OSM_MULTIPOLYGON_HPP

#include <sys/types.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <sstream>
#include <map>
#include <geos/io/WKBWriter.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>

#include "OsmWay.hpp"
#include "timer.h"

#define OUTPUT_SRID 4326

enum innerouter { UNSET, INNER, OUTER };
enum direction { NO_DIRECTION, CLOCKWISE, COUNTERCLOCKWISE };

using namespace std;
using namespace geos::geom;

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
    enum innerouter innerouter;
};
*/

struct WayInfo
{
    Osmium::OSM::Way *way;
    int used;
    int sequence;
    bool invert;
    int poly_id; 
    bool duplicate; 
    std::string errorhint;
    enum innerouter innerouter;
    enum innerouter orig_innerouter;
    WayInfo() { way = NULL; used = -1; poly_id=-1; innerouter = UNSET; orig_innerouter = UNSET; sequence = 0; invert = false; duplicate = false; }
    WayInfo(Osmium::OSM::Way *w, enum innerouter io) { way = w; orig_innerouter = io; used = -1; poly_id= -1; innerouter = UNSET; sequence = 0; invert = false;duplicate = false; }
};

struct RingInfo
{
    geos::geom::Polygon *polygon;
    enum direction direction;
    vector<WayInfo *> ways;
    vector<RingInfo *> inner_rings;
    bool nested;
    RingInfo *contained_by;
    RingInfo() { direction = NO_DIRECTION; contained_by = NULL; polygon = NULL; nested = false; ring_id = -1; }
    int ring_id;
};

namespace Osmium {

    namespace OSM {

        class Multipolygon {

            enum polygon_type_t { simple, multi } polygon_type;
            bool boundary;

          public:

            union {
                Way *way;
                Relation *relation;
            };

          public:

            /// the member ways of this multipolygon
            std::vector<Osmium::OSM::Way> member_ways;

            /// number of ways in this multipolygon
            int num_ways;

            /// how many ways are missing before we can build this multipolygon
            int missing_ways;

            geos::geom::Geometry *geometry;
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

            Multipolygon(Way *w) : polygon_type(simple), boundary(false), way(w) {
                init();
            }

            Multipolygon(Relation *r, bool b) : polygon_type(multi), boundary(b), relation(r) {
                num_ways = 0;
                init();
            }

            ~Multipolygon() {
                if (polygon_type == simple) {
                    // TODO
                } else {
                    delete relation;
                }
            }

            osm_object_id_t get_id() {
                if (polygon_type == multi) {
                    return relation->get_id();
                } else {
                    return way->get_id();
                }
            }

            bool build_geometry(Relation *r);

          private:

            RingInfo *make_one_ring(vector<WayInfo> &ways, osm_object_id_t first, osm_object_id_t last, int ringcount, int sequence);
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


        }; // class Multipolygon

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_MULTIPOLYGON_HPP
