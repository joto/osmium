#ifndef OSMIUM_OSM_MULTIPOLYGON_HPP
#define OSMIUM_OSM_MULTIPOLYGON_HPP

#ifdef WITH_GEOS
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTWriter.h>
#endif

#ifdef WITH_SHPLIB
#include <shapefil.h>
#endif

#ifdef WITH_MULTIPOLYGON_PROFILING
#include "timer.h"
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
            }

#ifdef WITH_GEOS
            Multipolygon(geos::geom::Geometry *geom) {
                geometry = geom;
            }
#endif

            ~Multipolygon() {
#ifdef WITH_GEOS
                if (geometry) delete(geometry);
#endif
            }

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
#endif

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

#ifdef WITH_MULTIPOLYGON_PROFILING
            static std::vector<std::pair<std::string, timer *> > timers;

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
#endif // WITH_MULTIPOLYGON_PROFILING

          public:

            MultipolygonFromRelation(Relation *r, bool b, int n, void (*callback)(Osmium::OSM::Multipolygon *), bool repair) : Multipolygon(), boundary(b), relation(r), callback(callback) {
                num_ways = n;
                missing_ways = n;
#ifdef WITH_GEOS
                geometry = NULL;
#endif // WITH_GEOS
                id = r->get_id();
                attempt_repair = repair;

#ifdef WITH_MULTIPOLYGON_PROFILING
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
#endif
            }

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

#ifdef WITH_MULTIPOLYGON_PROFILING
            static void print_timings() {
                for (unsigned int i=0; i<timers.size(); i++) {
                    std::cerr << timers[i].first << ": " << *(timers[i].second) << std::endl;
                }
            }
#endif

          private:

#ifdef WITH_GEOS
            bool build_geometry();

            RingInfo *make_one_ring(std::vector<WayInfo *> &ways, osm_object_id_t first, osm_object_id_t last, int ringcount, int sequence);
            bool find_and_repair_holes_in_rings(std::vector<WayInfo *> *ways);
            bool geometry_error(const char *message);
            geos::geom::LinearRing *create_non_intersecting_linear_ring(geos::geom::CoordinateSequence *cs);
#endif


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

        }; // class MultipolygonFromRelation

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_MULTIPOLYGON_HPP
