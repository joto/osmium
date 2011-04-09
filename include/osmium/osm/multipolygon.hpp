#ifndef OSMIUM_OSM_MULTIPOLYGON_HPP
#define OSMIUM_OSM_MULTIPOLYGON_HPP

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>

#ifdef WITH_MULTIPOLYGON_PROFILING
# include <osmium/utils/timer.h>
# define START_TIMER(x) x##_timer.start();
# define STOP_TIMER(x) x##_timer.stop();
#else
# define START_TIMER(x)
# define STOP_TIMER(x)
#endif

#ifdef WITH_GEOS
# include <geos/version.h>
# include <geos/geom/Geometry.h>
# include <geos/geom/Point.h>
# include <geos/geom/LineString.h>
# include <geos/geom/Polygon.h>
# include <geos/geom/PrecisionModel.h>
# include <geos/geom/CoordinateSequence.h>
# include <geos/geom/CoordinateArraySequenceFactory.h>
# include <geos/geom/LinearRing.h>
# include <geos/geom/Polygon.h>
# include <geos/geom/MultiPolygon.h>
# include <geos/geom/MultiLineString.h>
# include <geos/io/WKTWriter.h>
# include <geos/util/GEOSException.h>
# include <geos/opLinemerge.h>
# include <geos/operation/polygonize/Polygonizer.h>
# include <geos/operation/distance/DistanceOp.h>
# include <geos/opPolygonize.h>
# include <geos/algorithm/LineIntersector.h>
# include <geos/geomgraph/GeometryGraph.h>
# include <geos/geomgraph/index/SegmentIntersector.h>

// this should come from /usr/include/geos/algorithm, but its missing there in some Ubuntu versions
# include "../include/CGAlgorithms.h"
#endif

#ifdef WITH_SHPLIB
# include <shapefil.h>
#endif

namespace Osmium {

    namespace OSM {

        enum innerouter_t { UNSET, INNER, OUTER };
        enum direction_t { NO_DIRECTION, CLOCKWISE, COUNTERCLOCKWISE };

#ifdef WITH_GEOS
        class WayInfo {

            friend class MultipolygonFromRelation;

            Osmium::OSM::Way *way;
            int used;
            int sequence;
            bool invert;
            bool duplicate; 
            std::string errorhint;
            innerouter_t innerouter;
            innerouter_t orig_innerouter;
            geos::geom::Geometry *way_geom;
            int firstnode;
            int lastnode;
            bool tried;

            WayInfo() {
                way = NULL;
                used = -1;
                innerouter = UNSET;
                orig_innerouter = UNSET;
                sequence = 0;
                invert = false;
                duplicate = false;
                way_geom = NULL;
                firstnode = -1;
                lastnode = -1;
                tried = false;
            }

            WayInfo(Osmium::OSM::Way *w, innerouter_t io) {
                way = w;
                way_geom = w->create_geos_geometry();
                orig_innerouter = io;
                used = -1;
                innerouter = UNSET;
                sequence = 0;
                invert = false;
                duplicate = false;
                tried = false;
                firstnode = w->get_first_node_id();
                lastnode = w->get_last_node_id();
            }

            /** Special version with a synthetic way, not backed by real way object. */
            WayInfo(geos::geom::Geometry *g, int first, int last, innerouter_t io) {
                way = NULL;
                way_geom = g;
                orig_innerouter = io;
                used = -1;
                innerouter = UNSET;
                sequence = 0;
                invert = false;
                duplicate = false;
                tried = false;
                firstnode = first;
                lastnode = last;
            }

            ~WayInfo() {
                delete way_geom;
            }

            geos::geom::Point *get_firstnode_geom() {
                return (way ? way->get_first_node_geometry() : NULL);
            }

            geos::geom::Point *get_lastnode_geom() {
                return (way ? way->get_last_node_geometry() : NULL);
            }

        };

        class RingInfo {

            friend class MultipolygonFromRelation;

            geos::geom::Polygon *polygon;
            direction_t direction;
            std::vector<WayInfo *> ways;
            std::vector<RingInfo *> inner_rings;
            bool nested;
            RingInfo *contained_by;
            int ring_id;

            RingInfo() {
                polygon = NULL;
                direction = NO_DIRECTION;
                contained_by = NULL;
                nested = false;
                contained_by = NULL;
                ring_id = -1;
            }
        };
#endif

        /// virtual parent class for MultipolygonFromWay and MultipolygonFromRelation
        class Multipolygon : public Object {

          protected:

#ifdef WITH_GEOS
            geos::geom::Geometry *geometry;
#endif

            Multipolygon() {
#ifdef WITH_GEOS
                geometry = NULL;
#endif
#ifdef WITH_JAVASCRIPT
                js_tags_instance    = Osmium::Javascript::Template::create_tags_instance(this);
                js_object_instance  = Osmium::Javascript::Template::create_multipolygon_instance(this);
#endif
            }

#ifdef WITH_GEOS
            Multipolygon(geos::geom::Geometry *geom) {
                geometry = geom;
#ifdef WITH_JAVASCRIPT
                js_tags_instance    = Osmium::Javascript::Template::create_tags_instance(this);
                js_object_instance  = Osmium::Javascript::Template::create_multipolygon_instance(this);
#endif
            }
#endif

            ~Multipolygon() {
#ifdef WITH_GEOS
                delete(geometry);
#endif
            }

          public:

#ifdef WITH_JAVASCRIPT
            v8::Handle<v8::Value> js_get_from() const {
                const char *value = (get_type() == MULTIPOLYGON_FROM_WAY) ? "way" : "relation";
                return v8::String::New(value);
            }
#endif // WITH_JAVASCRIPT

        }; // class Multipolygon

        /***
        * Multipolygon created from a way (so strictly speaking this will
        * always be a simple polygon).
        * The way pointer given to the constructor will not be stored, all
        * needed attributes are copied.
        */
        class MultipolygonFromWay : public Multipolygon {

            osm_sequence_id_t num_nodes;
            double            lon[Osmium::OSM::Way::max_nodes_in_way];
            double            lat[Osmium::OSM::Way::max_nodes_in_way];

          public:

#ifdef WITH_GEOS
            MultipolygonFromWay(Way *way, geos::geom::Geometry *geom) : Multipolygon(geom) {
#else
            MultipolygonFromWay(Way *way) : Multipolygon() {
#endif // WITH_GEOS
                id        = way->get_id();
                version   = way->get_version();
                uid       = way->get_uid();
                changeset = way->get_changeset();
                timestamp = way->get_timestamp();

                num_tags  = way->tag_count();
                tags      = way->tags;

                num_nodes = way->node_count();
                for (int i=0; i < num_nodes; i++) {
                    lon[i] = way->lon[i];
                    lat[i] = way->lat[i];
                }
            }

            osm_object_type_t get_type() const {
                return MULTIPOLYGON_FROM_WAY;
            }

#ifdef WITH_SHPLIB
            /**
            * Create a SHPObject for this multipolygon and return it. You have to call
            * SHPDestroyObject() with this object when you are done.
            */
            SHPObject *create_shpobject(int shp_type) {
                if (shp_type != SHPT_POLYGON) {
                    throw std::runtime_error("a multipolygon can only be added to a shapefile of type polygon");
                }
                return SHPCreateSimpleObject(shp_type, num_nodes, lon, lat, NULL);
            }
#endif // WITH_SHPLIB

        }; // class MultipolygonFromWay

        /***
        * multipolygon created from a relation with tag type=multipolygon or type=boundary
        */
        class MultipolygonFromRelation : public Multipolygon {

            bool boundary; ///< was this multipolygon created from relation with tag type=boundary?

            /// the relation this multipolygon was build from
            Relation *relation;

            /// the member ways of this multipolygon
            std::vector<Osmium::OSM::Way> member_ways;

            /// number of ways in this multipolygon
            int num_ways;

            /// how many ways are missing before we can build this multipolygon
            int missing_ways;

            std::string geometry_error_message;

            /// callback we should call when a multipolygon was completed
            void (*callback)(Osmium::OSM::Multipolygon *);

            /// whether we want to repair a broken geometry
            bool attempt_repair;

            bool ignore_tag(const std::string &s) {
                if (s=="type") return true;
                if (s=="created_by") return true;
                if (s=="source") return true;
                if (s=="note") return true;
                return false;
            }

            bool same_tags(const Object *a, const Object *b) {
                if ((a == NULL) || (b == NULL)) return false;
                std::map<std::string, std::string> aTags;
                for (int i = 0; i < a->tag_count(); i++) {
                    if (ignore_tag(a->get_tag_key(i))) continue;
                    aTags[a->get_tag_key(i)] = a->get_tag_value(i);
                }
                for (int i = 0; i < b->tag_count(); i++) {
                    if (ignore_tag(b->get_tag_key(i))) continue;
                    if (aTags[b->get_tag_key(i)] != b->get_tag_value(i)) return false;
                    aTags.erase(b->get_tag_key(i));
                }
                if (!aTags.empty()) return false;
                return true;
            }

            /** returns false if there was a collision, true otherwise */
            bool merge_tags(Object *a, const Object *b) {
                bool rv = true;
                std::map<std::string, std::string> aTags;
                for (int i = 0; i < a->tag_count(); i++) {
                    if (ignore_tag(a->get_tag_key(i))) continue;
                    aTags[a->get_tag_key(i)] = a->get_tag_value(i);
                }
                for (int i = 0; i < b->tag_count(); i++) {
                    if (ignore_tag(b->get_tag_key(i))) continue;
                    if (aTags.find(b->get_tag_key(i)) != aTags.end()) {
                        if (aTags[b->get_tag_key(i)] != b->get_tag_value(i)) rv = false;
                    } else {
                        a->add_tag(b->get_tag_key(i), b->get_tag_value(i));
                        aTags[b->get_tag_key(i)] = b->get_tag_value(i);
                    }
                }
                return rv;
            }

            bool untagged(const Object *r) {
                if (r == NULL) return true;
                if (r->tag_count() == 0) return true;
                for (int i=0; i < r->tag_count(); i++) {
                    if (! ignore_tag(r->get_tag_key(i)) ) {
                        return false;
                    }
                }
                return true;
            }

          public:

            MultipolygonFromRelation(Relation *r, bool b, int n, void (*callback)(Osmium::OSM::Multipolygon *), bool repair) : Multipolygon(), boundary(b), relation(r), callback(callback) {
                num_ways = n;
                missing_ways = n;
#ifdef WITH_GEOS
                geometry = NULL;
#endif // WITH_GEOS
                id = r->get_id();
                attempt_repair = repair;
            }

#ifdef WITH_MULTIPOLYGON_PROFILING
            static std::vector<std::pair<std::string, timer *> > timers;

            static timer write_complex_poly_timer;
            static timer assemble_ways_timer;
            static timer assemble_nodes_timer;
            static timer make_one_ring_timer;
            static timer mor_polygonizer_timer;
            static timer mor_union_timer;
            static timer contains_timer;
            static timer extra_polygons_timer;
            static timer polygon_build_timer;
            static timer inner_ring_touch_timer;
            static timer polygon_intersection_timer;
            static timer multipolygon_build_timer;
            static timer multipolygon_write_timer;
            static timer error_write_timer;

            static void init_timings() {
                timers.push_back(std::pair<std::string, timer *> ("   thereof assemble_ways", &assemble_ways_timer));
                timers.push_back(std::pair<std::string, timer *> ("   thereof make_one_ring", &make_one_ring_timer));
                timers.push_back(std::pair<std::string, timer *> ("      thereof union", &mor_union_timer));
                timers.push_back(std::pair<std::string, timer *> ("      thereof polygonizer", &mor_polygonizer_timer));
                timers.push_back(std::pair<std::string, timer *> ("   thereof contains", &contains_timer));
                timers.push_back(std::pair<std::string, timer *> ("   thereof extra_polygons", &extra_polygons_timer));
                timers.push_back(std::pair<std::string, timer *> ("   thereof polygon_build", &polygon_build_timer));
                timers.push_back(std::pair<std::string, timer *> ("      thereof inner_ring_touch", &inner_ring_touch_timer));
                timers.push_back(std::pair<std::string, timer *> ("      thereof intersections", &polygon_intersection_timer));
                timers.push_back(std::pair<std::string, timer *> ("   thereof multipolygon_build", &multipolygon_build_timer));
                timers.push_back(std::pair<std::string, timer *> ("   thereof multipolygon_write", &multipolygon_write_timer));
                timers.push_back(std::pair<std::string, timer *> ("   thereof error_write", &error_write_timer));
            }

            static void print_timings() {
                for (unsigned int i=0; i<timers.size(); i++) {
                    std::cerr << timers[i].first << ": " << *(timers[i].second) << std::endl;
                }
            }
#endif

            ~MultipolygonFromRelation() {
                delete relation;
                member_ways.erase(member_ways.begin(), member_ways.end());
            }

            osm_object_type_t get_type() const {
                return MULTIPOLYGON_FROM_RELATION;
            }

            /// Add way to list of member ways. This will create a copy of the way.
            void add_member_way(Osmium::OSM::Way *way) {
                member_ways.push_back(*way);
                missing_ways--;
            }

            /// Do we have all the ways we need to build this multipolygon?
            bool is_complete() {
                return missing_ways == 0;
            }

            void handle_complete_multipolygon() {
                // std::cerr << "MP multi multi=" << id << " done\n";

#ifdef WITH_GEOS
                if (build_geometry()) {
                    geos::io::WKTWriter wkt;
                    //std::cerr << "  mp geometry: " << wkt.write(geometry) << std::endl;
                } else {
                    std::cerr << "  geom build error: " << geometry_error_message << "\n";
                }
#endif // WITH_GEOS

                callback(this);
            }

            /**
            * Create a SHPObject for this multipolygon and return it. You have to call
            * SHPDestroyObject() with this object when you are done.
            * Returns NULL if a valid SHPObject could not be created.
            */
#ifdef WITH_SHPLIB
            SHPObject *create_shpobject(int shp_type) {
                if (shp_type != SHPT_POLYGON) {
                    throw std::runtime_error("a multipolygon can only be added to a shapefile of type polygon");
                }

                if (!geometry) {
                    return NULL;
                }

                std::vector<double> x;
                std::vector<double> y;
                std::vector<int> partStart;

                dump_geometry(geometry, partStart, x, y);

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

                delete[] ps;
                delete[] xx;
                delete[] yy;
                
                return o;
            }
#endif

          private:

#ifdef WITH_SHPLIB
#ifdef WITH_GEOS
            bool dump_geometry(const geos::geom::Geometry *g, std::vector<int>& partStart, std::vector<double>& x, std::vector<double>& y) {
                switch(g->getGeometryTypeId()) {
                    case geos::geom::GEOS_MULTIPOLYGON:
                    case geos::geom::GEOS_MULTILINESTRING: {
                        for (size_t i=0; i<g->getNumGeometries(); i++) {
                            if (!dump_geometry(g->getGeometryN(i), partStart, x, y)) return false;
                        }
                        break;
                    }
                    case geos::geom::GEOS_POLYGON: {
                        geos::geom::Polygon *p = (geos::geom::Polygon *) g;
                        if (!dump_geometry(p->getExteriorRing(), partStart, x, y)) return false;
                        for (size_t i=0; i<p->getNumInteriorRing(); i++)
                        {
                            if (!dump_geometry(p->getInteriorRingN(i), partStart, x, y)) return false; 
                        }
                        break;
                    }
                    case geos::geom::GEOS_LINESTRING:
                    case geos::geom::GEOS_LINEARRING: {
                        partStart.push_back(x.size());
                        const geos::geom::CoordinateSequence *cs = ((geos::geom::LineString *) g)->getCoordinatesRO();
                        for (size_t i = 0; i < cs->getSize(); i++) {
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
#endif
#endif

            /**
            * This helper gets called when we find a ring that is not valid - 
            * usually because it self-intersects. The method tries to salvage
            * as much of the ring as possible, using binary search to find the
            * bit that needs to be cut out. It then returns a valid LinearRing,
            * or NULL if none can be built.
            *
            * There is massive potential for improvement here. The biggest
            * limitation is that this method does not deliver results for 
            * linear rings with more than one self-intersection.
            */
#ifdef WITH_GEOS
            geos::geom::LinearRing *create_non_intersecting_linear_ring(geos::geom::CoordinateSequence *orig_cs)
            {
                const std::vector<geos::geom::Coordinate>* coords = orig_cs->toVector();
                int inv = coords->size();
                int val = 0;
                int current = (inv + val) / 2;
                bool simple;

                // find the longest non-intersecting stretch from the beginning 
                // of the way.
                while(1)
                {
                    std::vector<geos::geom::Coordinate> *vv = new std::vector<geos::geom::Coordinate>(coords->begin(), coords->begin() + current);
                    geos::geom::CoordinateSequence *cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(vv);
                    geos::geom::LineString *a = Osmium::global.geos_geometry_factory->createLineString(cs);
                    if (!(simple = a->isSimple()))
                    {
                        inv = current;
                    }
                    else
                    {
                        val = current;
                    }
                    delete a;
                    if (current == (inv+val)/2) break;
                    current = (inv + val) / 2;
                }

                if (!simple) current--;

                unsigned int cutoutstart = current;

                inv = 0;
                val = coords->size();
                current = (inv + val) / 2;

                // find the longest non-intersecting stretch from the end
                // of the way. Note that this is likely to overlap with the
                // stretch found above - assume a 10-node way where nodes 3 
                // and 7 are identical, then we will find the sequence 0..6
                // above, and 4..9 here!

                while(1)
                {
                    std::vector<geos::geom::Coordinate> *vv = new std::vector<geos::geom::Coordinate>(coords->begin() + current, coords->end());
                    geos::geom::CoordinateSequence *cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(vv);
                    geos::geom::LineString *a = Osmium::global.geos_geometry_factory->createLineString(cs);
                    if (!(simple = a->isSimple()))
                    {
                        inv = current;
                    }
                    else
                    {
                        val = current;
                    }
                    delete a;
                    if (current == (inv+val)/2) break;
                    current = (inv + val) / 2;
                }
                if (!simple) current++;
                unsigned int cutoutend = current;

                // assemble a new linear ring by cutting out the problematic bit.
                // if the "problematic bit" however is longer than half the way,
                // then try using the "problematic bit" by itself.

                std::vector<geos::geom::Coordinate> *vv = new std::vector<geos::geom::Coordinate>();
                if (cutoutstart<cutoutend) { unsigned int t = cutoutstart; cutoutstart=cutoutend; cutoutend=t; }
                if (cutoutstart-cutoutend > coords->size() / 2)
                {
                    vv->insert(vv->end(), coords->begin() + cutoutend, coords->begin() + cutoutstart);
                    vv->insert(vv->end(), vv->at(0));
                }
                else
                {
                    vv->insert(vv->end(), coords->begin(), coords->begin() + cutoutend);
                    vv->insert(vv->end(), coords->begin() + cutoutstart, coords->end());
                }
                geos::geom::CoordinateSequence *cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(vv);
                geos::geom::LinearRing *a = Osmium::global.geos_geometry_factory->createLinearRing(cs);

                // if this results in a valid ring, return it; else return NULL.

                if (!a->isValid()) return NULL;
                geos::geom::LinearRing *b = (geos::geom::LinearRing *) a->clone();
                //delete a;
                return b;
            }

            /**
            * Tries to collect 1...n ways from the n ways in the given list so that
            * they form a closed ring. If this is possible, flag those as being used
            * by ring #ringcount in the way list and return the geometry. (The method
            * may be called again to find further rings.) If this is not possible, 
            * return NULL.
            */
            RingInfo *make_one_ring(std::vector<WayInfo *> &ways, osm_object_id_t first, osm_object_id_t last, int ringcount, int sequence)
            {

                // have we found a loop already?
                if (first && first == last)
                {
                    geos::geom::CoordinateSequence *cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(0, 0);
                    geos::geom::LinearRing *lr = NULL;
                    try
                    {
                        START_TIMER(mor_polygonizer);
                        WayInfo **sorted_ways = new WayInfo*[sequence];
                        for (unsigned int i=0; i<ways.size(); i++)
                        {
                            if (ways[i]->used == ringcount) 
                            {
                                sorted_ways[ways[i]->sequence] = ways[i];
                            }
                        }
                        for (int i=0; i<sequence; i++)
                        {
                            cs->add(((geos::geom::LineString *)sorted_ways[i]->way_geom)->getCoordinatesRO(), false, !sorted_ways[i]->invert);
                        }
                        delete[] sorted_ways;
                        lr = Osmium::global.geos_geometry_factory->createLinearRing(cs);
                        STOP_TIMER(mor_polygonizer);
                        if (!lr->isSimple() || !lr->isValid())
                        { 
                            //delete lr;
                            lr = NULL;
                            if (attempt_repair)
                            {
                                lr = create_non_intersecting_linear_ring(cs);
                                if (lr)
                                {
                                    if (Osmium::global.debug) 
                                        std::cerr << "successfully repaired an invalid ring" << std::endl;
                                }
                            }
                            if (!lr) return NULL; 
                        }
                        bool ccw = geos::algorithm::CGAlgorithms::isCCW(lr->getCoordinatesRO());
                        RingInfo *rl = new RingInfo();
                        rl->direction = ccw ? COUNTERCLOCKWISE : CLOCKWISE;
                        rl->polygon = Osmium::global.geos_geometry_factory->createPolygon(lr, NULL);
                        return rl;
                    }
                    catch (const geos::util::GEOSException& exc) 
                    {
                        if (Osmium::global.debug)
                            std::cerr << "Exception: " << exc.what() << std::endl;
                        return NULL;
                    }
                }

                // have we not allocated anything yet, then simply start with first available way, 
                // or return NULL if all are taken.
                if (!first)
                {
                    for (unsigned int i=0; i<ways.size(); i++)
                    {
                        if (ways[i]->used != -1) continue;
                        ways[i]->used = ringcount;
                        ways[i]->sequence = 0;
                        ways[i]->invert = false;
                        RingInfo *rl = make_one_ring(ways, ways[i]->firstnode, ways[i]->lastnode, ringcount, 1);
                        if (rl)
                        {
                            rl->ways.push_back(ways[i]);
                            return rl;
                        }
                        ways[i]->used = -2;
                        break;
                    }
                    return NULL;
                }

                // try extending our current line at the rear end
                // since we are looking for a LOOP, no sense to try extending it at both ends 
                // as we'll eventually get there anyway!

                for (unsigned int i=0; i<ways.size(); i++)
                {
                    if (ways[i]->used < 0) ways[i]->tried = false;
                }

                for (unsigned int i=0; i<ways.size(); i++)
                {
                    // ignore used ways
                    if (ways[i]->used >= 0) continue;
                    if (ways[i]->tried) continue;
                    ways[i]->tried = true;

                    int old_used = ways[i]->used;
                    if (ways[i]->firstnode == last)
                    {
                        // add way to end
                        ways[i]->used = ringcount;
                        ways[i]->sequence = sequence;
                        ways[i]->invert = false;
                        RingInfo *result = make_one_ring(ways, first, ways[i]->lastnode, ringcount, sequence+1);
                        if (result) { result->ways.push_back(ways[i]); return result; }
                        ways[i]->used = old_used;
                    }
                    else if (ways[i]->lastnode == last)
                    {
                        // add way to end, but turn it around
                        ways[i]->used = ringcount;
                        ways[i]->sequence = sequence;
                        ways[i]->invert = true;
                        RingInfo *result = make_one_ring(ways, first, ways[i]->firstnode, ringcount, sequence+1);
                        if (result) { result->ways.push_back(ways[i]); return result; }
                        ways[i]->used = old_used;
                    }
                }
                // we have exhausted all combinations.
                return NULL;
            }

            /**
            * Checks if there are any dangling ends, and connects them to the 
            * nearest other dangling end with a straight line. This could 
            * conceivably introduce intersections, but it's the best we can
            * do.
            *
            * Returns true on success.
            *
            * (This implementation always succeeds because it is impossible for 
            * there to be only one dangling end in a collection of lines.)
            */
            bool find_and_repair_holes_in_rings(std::vector<WayInfo *> *ways)
            { 
                // collect the remaining debris (=unused ways) and find dangling nodes.
                
                std::map<int,geos::geom::Point *> dangling_node_map;
                for (std::vector<WayInfo *>::iterator i = ways->begin(); i != ways->end(); i++)
                {       
                    if ((*i)->used < 0)
                    {
                        (*i)->innerouter = UNSET;
                        (*i)->used = -1;
                        for (int j=0; j<2; j++)
                        {
                            int nid = j ? (*i)->firstnode : (*i)->lastnode;
                            if (dangling_node_map[nid])
                            {
                                delete dangling_node_map[nid];
                                dangling_node_map[nid] = NULL;
                            }
                            else
                            {
                                dangling_node_map[nid] = j ? (*i)->get_firstnode_geom() : (*i)->get_lastnode_geom();
                            }
                        }
                    }
                }

                do
                {
                    int mindist_id = 0;
                    double mindist = -1;
                    int node1_id = 0;
                    geos::geom::Point *node1 = NULL;
                    geos::geom::Point *node2 = NULL;

                    // find one pair consisting of a random node from the list (node1)
                    // plus the node that lies closest to it.
                    for (std::map<int,geos::geom::Point *>::iterator i = dangling_node_map.begin(); i!= dangling_node_map.end(); i++)
                    {
                        if (!i->second) continue;
                        if (node1 == NULL)
                        {
                            node1 = i->second;
                            node1_id = i->first;
                            i->second = NULL;
                            mindist = -1;
                        }
                        else
                        {
#if GEOS_VERSION_MAJOR < 3 || (GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR <= 2)
                            double dist = geos::operation::distance::DistanceOp::distance(node1, (i->second)); // deprecated in newer version of GEOS
#else
                            double dist = geos::operation::distance::DistanceOp::distance(*node1, *(i->second));
#endif
                            if ((dist < mindist) || (mindist < 0))
                            {
                                mindist = dist;
                                mindist_id = i->first;
                            }
                        }
                    }

                    // if such a pair has been found, synthesize a connecting way.
                    if (node1 && mindist > -1)
                    {
                        // if we find that there are dangling nodes but aren't 
                        // repairing - break out.
                        if (!attempt_repair) return false;

                        // drop node2 from dangling map
                        node2 = dangling_node_map[mindist_id];
                        dangling_node_map[mindist_id] = NULL;

                        std::vector<geos::geom::Coordinate> *c = new std::vector<geos::geom::Coordinate>;
                        c->push_back(*(node1->getCoordinate()));
                        c->push_back(*(node2->getCoordinate()));
                        geos::geom::CoordinateSequence *cs = Osmium::global.geos_geometry_factory->getCoordinateSequenceFactory()->create(c);
                        geos::geom::Geometry *geometry = (geos::geom::Geometry *) Osmium::global.geos_geometry_factory->createLineString(cs);
                        ways->push_back(new WayInfo(geometry, node1_id, mindist_id, UNSET));
                        if (Osmium::global.debug) 
                            std::cerr << "fill gap between nodes " << node1_id << " and " << mindist_id << std::endl;
                    }
                    else
                    {
                        break;
                    }
                } while(1);

                return true;
            }


            /**
            * Tries to build a multipolygon from the given relation.
            *
            */
            bool build_geometry()
            {
                std::vector<WayInfo *> ways;

                // the timestamp of the multipolygon will be the maximum of the timestamp from the relation and from all member ways
                time_t timestamp = relation->get_timestamp();

                // assemble all ways which are members of this relation into a 
                // vector of WayInfo elements. this holds room for the way pointer
                // and some extra flags.
                
                START_TIMER(assemble_ways);
                for (std::vector<Way>::iterator i = member_ways.begin(); i != member_ways.end(); i++) 
                {
                    if (i->get_timestamp() > timestamp) timestamp = i->get_timestamp();
                    WayInfo *wi = new WayInfo(&(*i), UNSET);
                    if (wi->way_geom) 
                    {
                        geos::io::WKTWriter wkt;
                    } 
                    else 
                    {
                        delete wi;
                        return geometry_error("invalid way geometry in multipolygon relation member");
                    }
                    ways.push_back(wi);
                    // TODO drop duplicate ways automatically in repair mode?
                    // TODO maybe add INNER/OUTER instead of UNSET to enable later warnings on role mismatch
                }
                STOP_TIMER(assemble_ways);

                std::vector<RingInfo *> ringlist;

                // convenience defines to aid in clearing up on error return.
                #define clear_ringlist() \
                for (std::vector<RingInfo *>::const_iterator rli = ringlist.begin(); rli != ringlist.end(); rli++) delete *rli;
                #define clear_wayinfo() \
                for (std::vector<WayInfo *>::const_iterator win = ways.begin(); win != ways.end(); win++) delete *win;

                // try and create as many closed rings as possible from the assortment
                // of ways. make_one_ring will automatically flag those that have been
                // used so they are not used again.
                
                do 
                {
                    START_TIMER(make_one_ring);
                    RingInfo *r = make_one_ring(ways, 0, 0, ringlist.size(), 0);
                    STOP_TIMER(make_one_ring);
                    if (r == NULL) break;
                    r->ring_id = ringlist.size();
                    ringlist.push_back(r);
                } 
                while(1);

                if (ringlist.empty())
                {
                    // FIXME return geometry_error("no rings");
                }

                if (!find_and_repair_holes_in_rings(&ways))
                {
                    clear_ringlist();
                    clear_wayinfo();
                    return geometry_error("un-connectable dangling ends");
                }

                // re-run ring building, taking into account the newly created "repair" bits.
                // (in case there were no dangling bits, make_one_ring terminates quickly.)
                do 
                {
                    START_TIMER(make_one_ring);
                    RingInfo *r = make_one_ring(ways, 0, 0, ringlist.size(), 0);
                    STOP_TIMER(make_one_ring);
                    if (r == NULL) break;
                    r->ring_id = ringlist.size();
                    ringlist.push_back(r);
                } 
                while(1);

                if (ringlist.empty())
                {
                    clear_ringlist();
                    clear_wayinfo();
                    return geometry_error("no rings");
                }

                std::vector<geos::geom::Geometry *> *polygons = new std::vector<geos::geom::Geometry *>();

                geos::geom::MultiPolygon *mp = NULL;

                // find out which ring contains which other ring, so we know
                // which are inner rings and which outer. don't trust the "role"
                // specifications.

                START_TIMER(contains);

                bool **contains = new bool*[ringlist.size()];
                bool *contained_by_even_number = new bool[ringlist.size()];

                // reset array
                for (unsigned int i=0; i<ringlist.size(); i++)
                {
                    contains[i] = new bool[ringlist.size()];
                    contained_by_even_number[i] = true;
                    for (unsigned int j=0; j<ringlist.size(); j++)
                    {
                        contains[i][j] = false;
                    }
                }

                // build contains relationships.
                // we use contained_by_even_number as a helper for us to determine
                // whether something is an inner (false) or outer (true) ring.

                for (unsigned int i=0; i<ringlist.size(); i++)
                {
                    for (unsigned int j=0; j<ringlist.size(); j++)
                    {
                        if (i==j) continue;
                        if (contains[j][i]) continue;
                        contains[i][j] = ringlist[i]->polygon->contains(ringlist[j]->polygon);
                        contained_by_even_number[j] ^= contains[i][j];
                    }
                }

                // we now have an array that has a true value whenever something is 
                // contained by something else; if a contains b and b contains c, then
                // our array says that a contains b, b contains c, and a contains c.
                // thin out the array so that only direct relationships remain (and
                // the "a contains c" is dropped).

                for (unsigned int i=0; i<ringlist.size(); i++)
                {
                    for (unsigned j=0; j<ringlist.size(); j++)
                    {
                        if (contains[i][j]) 
                        {
                            // see if there is an intermediary relationship
                            for (unsigned int k=0; k<ringlist.size(); k++)
                            {
                                if (k==i) continue;
                                if (k==j) continue;
                                if (contains[i][k] && contains[k][j])
                                {
                                    // intermediary relationship exists; break this
                                    // one up.
                                    contains[i][j] = false;
                                    ringlist[j]->nested = true;
                                    break;
                                }
                            }
                        }
                    }
                }

                // populate the "inner_rings" list and the "contained_by" pointer
                // in the ring list based on the data collected. the "contains"
                // array can be thrown away afterwards.

                for (unsigned int i=0; i<ringlist.size(); i++)
                {
                    for (unsigned int j=0; j<ringlist.size(); j++)
                    {
                        if (contains[i][j] && !contained_by_even_number[j])
                        {
                            ringlist[j]->contained_by = ringlist[i];
                            ringlist[i]->inner_rings.push_back(ringlist[j]);
                        }
                    }
                    delete[] contains[i];
                }

                delete[] contains;
                delete[] contained_by_even_number;
                STOP_TIMER(contains);

                // now look at all enclosed (inner) rings that consist of only one way.
                // if such an inner ring has way tags, do the following:
                // * emit an extra polygon for the inner ring if the tags are different 
                //   from the relation's
                // * emit a warning, and ignore the inner ring, if the tags are the same
                //   as for the relation

                START_TIMER(extra_polygons);
                for (unsigned int i=0; i<ringlist.size(); i++)
                {
                    if (ringlist[i]->contained_by)
                    {
                        if (ringlist[i]->ways.size() == 1 && !untagged(ringlist[i]->ways[0]->way))
                        {
                            std::vector<geos::geom::Geometry *> *g = new std::vector<geos::geom::Geometry *>;
                            if (ringlist[i]->direction == CLOCKWISE)
                            {
                                g->push_back(ringlist[i]->polygon->clone());
                            }
                            else
                            {
                                geos::geom::LineString *tmp = (geos::geom::LineString *) ringlist[i]->polygon->getExteriorRing()->reverse();
                                geos::geom::LinearRing *reversed_ring = 
                                Osmium::global.geos_geometry_factory->createLinearRing(tmp->getCoordinates());
                                delete tmp;
                                g->push_back(Osmium::global.geos_geometry_factory->createPolygon(reversed_ring, NULL));
                            }

                            geos::geom::MultiPolygon *special_mp = Osmium::global.geos_geometry_factory->createMultiPolygon(g);

                            if (same_tags(ringlist[i]->ways[0]->way, relation))
                            {
                                // warning
                                // warnings.insert("duplicate_tags_on_inner");
                            }
                            else if (ringlist[i]->contained_by->ways.size() == 1 && same_tags(ringlist[i]->ways[0]->way, ringlist[i]->contained_by->ways[0]->way))
                            {
                                // warning
                                // warnings.insert("duplicate_tags_on_inner");
                            }
                            else
                            {
                                Osmium::OSM::MultipolygonFromWay *internal_mp =
                                    new Osmium::OSM::MultipolygonFromWay(ringlist[i]->ways[0]->way, special_mp);
                                callback(internal_mp);
                                delete internal_mp;
                                // MultipolygonFromWay destructor deletes the
                                // geometry, so avoid to delete it again.
                                special_mp = NULL;
                            }
                            delete special_mp;
                        }
                    }
                }
                STOP_TIMER(extra_polygons);

                // for all non-enclosed rings, assemble holes and build polygon.

                START_TIMER(polygon_build)
                for (unsigned int i=0; i<ringlist.size(); i++)
                {
                    // look only at outer, i.e. non-contained rings. each ends up as one polygon.
                    if (ringlist[i] == NULL) continue; // can happen if ring has been deleted
                    if (ringlist[i]->contained_by) continue;

                    std::vector<geos::geom::Geometry *> *holes = new std::vector<geos::geom::Geometry *>(); // ownership is later transferred to polygon

                    START_TIMER(inner_ring_touch)
                    for (int j=0; j<((int)ringlist[i]->inner_rings.size()-1); j++)
                    {
                        if (!ringlist[i]->inner_rings[j]->polygon) continue;
                        geos::geom::LinearRing *ring = (geos::geom::LinearRing *) ringlist[i]->inner_rings[j]->polygon->getExteriorRing();

                        // check if some of the rings touch another ring.

                        for (unsigned int k=j + 1; k<ringlist[i]->inner_rings.size(); k++)
                        {
                            if (!ringlist[i]->inner_rings[k]->polygon) continue;
                            const geos::geom::Geometry *compare = ringlist[i]->inner_rings[k]->polygon->getExteriorRing();
                            geos::geom::Geometry *inter = NULL;
                            try 
                            {
                                if (!ring->intersects(compare)) continue;
                                inter = ring->intersection(compare);
                            }
                            catch (const geos::util::GEOSException& exc) 
                            {
                                // nop;
                            }
                            if (inter && (inter->getGeometryTypeId() == geos::geom::GEOS_LINESTRING || inter->getGeometryTypeId() == geos::geom::GEOS_MULTILINESTRING))
                            {
                                // touching inner rings
                                // this is allowed, but we must fix them up into a valid
                                // geometry
                                geos::geom::Geometry *diff = ring->symDifference(compare);
                                geos::operation::polygonize::Polygonizer *p = new geos::operation::polygonize::Polygonizer();
                                p->add(diff);
                                std::vector<geos::geom::Polygon*>* polys = p->getPolygons();
                                if (polys && polys->size() == 1)
                                {
                                    ringlist[i]->inner_rings[j]->polygon = polys->at(0);
                                    bool ccw = geos::algorithm::CGAlgorithms::isCCW(polys->at(0)->getExteriorRing()->getCoordinatesRO());
                                    ringlist[i]->inner_rings[j]->direction = ccw ? COUNTERCLOCKWISE : CLOCKWISE;
                                    ringlist[i]->inner_rings[k]->polygon = NULL;
                                    j=-1; break;
                                }
                            }
                            else
                            {
                                // other kind of intersect between inner rings; this is
                                // not allwoed and will lead to an exception later when
                                // building the MP
                            }
                        }
                    }
                    STOP_TIMER(inner_ring_touch)

                    for (unsigned int j=0; j<ringlist[i]->inner_rings.size(); j++)
                    {
                        if (!ringlist[i]->inner_rings[j]->polygon) continue;
                        geos::geom::LinearRing *ring = (geos::geom::LinearRing *) ringlist[i]->inner_rings[j]->polygon->getExteriorRing();

                        if (ringlist[i]->inner_rings[j]->direction == CLOCKWISE)
                        {
                            // reverse ring
                            geos::geom::LineString *tmp = (geos::geom::LineString *) ring->reverse();
                            geos::geom::LinearRing *reversed_ring = 
                                Osmium::global.geos_geometry_factory->createLinearRing(tmp->getCoordinates());
                            delete tmp;
                            holes->push_back(reversed_ring);
                        }
                        else
                        {
                            holes->push_back(ring);
                        }
                    }

                    geos::geom::LinearRing *ring = (geos::geom::LinearRing *) ringlist[i]->polygon->getExteriorRing();
                    if (ringlist[i]->direction == COUNTERCLOCKWISE)
                    {
                        geos::geom::LineString *tmp = (geos::geom::LineString *) ring->reverse();
                        geos::geom::LinearRing *reversed_ring = Osmium::global.geos_geometry_factory->createLinearRing(tmp->getCoordinates());
                        ring = reversed_ring;
                        delete tmp;
                    }
                    else
                    {
                        ring = (geos::geom::LinearRing *) ring->clone();
                    }
                    delete ringlist[i]->polygon;
                    ringlist[i]->polygon = NULL;
                    geos::geom::Polygon *p = NULL;
                    bool valid = false;

                    try
                    {
                        p = Osmium::global.geos_geometry_factory->createPolygon(ring, holes);
                        if (p) valid = p->isValid();
                    }
                    catch (const geos::util::GEOSException& exc) 
                    {
                        // nop
                        if (Osmium::global.debug)
                            std::cerr << "Exception during creation of polygon for relation #" << relation->id << ": " << exc.what() << " (treating as invalid polygon)" << std::endl;
                    }
                    if (!valid)
                    {
                        // polygon is invalid.
                        clear_ringlist();
                        clear_wayinfo();
                        if (p) delete p; else delete ring;
                        return geometry_error("invalid ring");
                    }
                    else
                    {
                        polygons->push_back(p);
                        for (unsigned int k=0; k<ringlist[i]->ways.size(); k++)
                        {       
                            WayInfo *wi = ringlist[i]->ways[k];
                            // may have "hole filler" ways in there, not backed by
                            // proper way and thus no tags:
                            if (wi->way == NULL) continue;
                            if (untagged(wi->way))
                            {
                                // way not tagged - ok
                            }
                            else if (same_tags(relation, wi->way))
                            {
                                // way tagged the same as relation/previous ways, ok
                            }
                            else if (untagged(relation))
                            {
                                // relation untagged; use tags from way; ok
                                merge_tags(relation, wi->way);
                            }

                            wi->innerouter = OUTER;
                            if (wi->orig_innerouter == INNER && wi->errorhint.empty()) 
                            { 
                                // warning: inner/outer mismatch
                            }
                        }
                        // copy tags from relation into multipolygon
                        num_tags = relation->tag_count();
                        tags = relation->tags;
                    }
                    // later delete ringlist[i];
                    // ringlist[i] = NULL;
                }
                STOP_TIMER(polygon_build);

                clear_ringlist();
                clear_wayinfo();
                if (polygons->empty()) 
                {
                    return geometry_error("no rings");
                }

                START_TIMER(multipolygon_build);
                bool valid = false;
                try
                {
                    mp = Osmium::global.geos_geometry_factory->createMultiPolygon(polygons);
                    valid = mp->isValid();
                }
                catch (const geos::util::GEOSException& exc)
                {
                    // nop
                };
                STOP_TIMER(multipolygon_build);
                if (valid)
                {
                    geometry = mp;
                    return true;
                }
                return geometry_error("multipolygon invalid");
            }

            bool geometry_error(const char *message)
            {
                geometry_error_message = message;
                if (Osmium::global.debug)
                    std::cerr << "building mp failed: " << geometry_error_message << std::endl;
                geometry = NULL;
                return false;
            }
#endif // WITH_GEOS

        }; // class MultipolygonFromRelation

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_MULTIPOLYGON_HPP
