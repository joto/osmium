#ifndef OSMIUM_MULTIPOLYGON_BUILDER_HPP
#define OSMIUM_MULTIPOLYGON_BUILDER_HPP

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

#include <assert.h>
#include <vector>
#include <map>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>
using boost::make_shared;

#ifdef OSMIUM_WITH_MULTIPOLYGON_PROFILING
# include <osmium/utils/timer.h>
# define START_TIMER(x) x##_timer.start();
# define STOP_TIMER(x) x##_timer.stop();
#else
# define START_TIMER(x)
# define STOP_TIMER(x)
#endif // OSMIUM_WITH_MULTIPOLYGON_PROFILING

#include <geos/version.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/util/GEOSException.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/distance/DistanceOp.h>

// this should come from /usr/include/geos/algorithm, but its missing there in some Ubuntu versions
#include "../CGAlgorithms.h"

#include <osmium/osm/way.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/geometry.hpp>
#include <osmium/geometry/linestring.hpp>
#include <osmium/geometry/geos.hpp>
#include <osmium/relations/relation_info.hpp>

namespace Osmium {

    namespace MultiPolygon {

        struct BuildError : public std::runtime_error {
            BuildError(const std::string& what) :
                std::runtime_error(what) {
            }
        };

        struct InvalidWayGeometry : public BuildError {
            InvalidWayGeometry(const std::string& what) :
                BuildError(what) {
            }
        };

        struct DanglingEnds : public BuildError {
            DanglingEnds(const std::string& what) :
                BuildError(what) {
            }
        };

        struct NoRings : public BuildError {
            NoRings(const std::string& what) :
                BuildError(what) {
            }
        };

        struct InvalidRing : public BuildError {
            InvalidRing(const std::string& what) :
                BuildError(what) {
            }
        };

        struct InvalidMultiPolygon : public BuildError {
            InvalidMultiPolygon(const std::string& what) :
                BuildError(what) {
            }
        };

        enum innerouter_t { UNSET, INNER, OUTER };
        enum direction_t { NO_DIRECTION, CLOCKWISE, COUNTERCLOCKWISE };

        class WayInfo {

            friend class Builder;

            const Osmium::OSM::Way* way;
            int used;
            int sequence;
            bool invert;
            innerouter_t innerouter;
            innerouter_t orig_innerouter;
            geos::geom::Geometry* way_geom;
            int firstnode;
            int lastnode;
            bool tried;

            WayInfo() :
                way(NULL),
                used(-1),
                sequence(0),
                invert(false),
                innerouter(UNSET),
                orig_innerouter(UNSET),
                way_geom(NULL),
                firstnode(-1),
                lastnode(-1),
                tried(false) {
            }

            WayInfo(const Osmium::OSM::Way* w, innerouter_t io) :
                way(w),
                used(-1),
                sequence(0),
                invert(false),
                innerouter(UNSET),
                orig_innerouter(io),
                way_geom(NULL),
                firstnode(w->get_first_node_id()),
                lastnode(w->get_last_node_id()),
                tried(false) {
                Osmium::Geometry::LineString linestring(*w);
                way_geom = Osmium::Geometry::create_geos_geometry(linestring);
            }

            /** Special version with a synthetic way, not backed by real way object. */
            WayInfo(geos::geom::Geometry* geom, int first, int last, innerouter_t io) :
                way(NULL),
                used(-1),
                sequence(0),
                invert(false),
                innerouter(UNSET),
                orig_innerouter(io),
                way_geom(geom),
                firstnode(first),
                lastnode(last),
                tried(false) {
            }

            ~WayInfo() {
                delete way_geom;
            }

            geos::geom::Point* get_firstnode_geom() const {
                if (!way) {
                    return NULL;
                }
                return Osmium::Geometry::geos_geometry_factory()->createPoint(Osmium::Geometry::create_geos_coordinate(way->nodes().front().position()));
            }

            geos::geom::Point* get_lastnode_geom() const {
                if (!way) {
                    return NULL;
                }
                return Osmium::Geometry::geos_geometry_factory()->createPoint(Osmium::Geometry::create_geos_coordinate(way->nodes().back().position()));
            }

        }; // class WayInfo

        class RingInfo {

            friend class Builder;

            geos::geom::Polygon* polygon;
            direction_t direction;
            std::vector<WayInfo*> ways;
            std::vector<RingInfo*> inner_rings;
            RingInfo* contained_by;

            RingInfo(geos::geom::Polygon* p, direction_t dir) :
                polygon(p),
                direction(dir),
                ways(),
                inner_rings(),
                contained_by(NULL) {
            }

        }; // class RingInfo

        /**
         *
         */
        class Builder {

            /// Relation information (including members) to build the area from.
            const Osmium::Relations::RelationInfo& m_relation_info;

            /// All areas generated will end up in this vector.
            std::vector< shared_ptr<Osmium::OSM::Area> > m_areas;

            /// Do we want to attempt repair of a broken geometry?
            const bool m_attempt_repair;

            /// This is the new area we are building.
            shared_ptr<Osmium::OSM::Area> m_new_area;

            /**
             * Return true if the given tag key is in a fixed list of keys we are
             * not interested in.
             */
            bool ignore_tag(const std::string& key) const {
                if (key == "type") return true;
                if (key == "created_by") return true;
                if (key == "source") return true;
                if (key == "note") return true;
                return false;
            }

            /**
             * Compare tags on two OSM objects ignoring tags with certain keys
             * defined in the ignore_tag() method.
             *
             * @returns true if all tags are the same, false otherwise.
             */
            bool same_tags(const Osmium::OSM::Object* a, const Osmium::OSM::Object* b) const {
                if ((a == NULL) || (b == NULL)) return false;

                std::map<std::string, std::string> tag_map;

                BOOST_FOREACH(const Osmium::OSM::Tag& tag, a->tags()) {
                    if (!ignore_tag(tag.key())) {
                        tag_map[tag.key()] = tag.value();
                    }
                }

                BOOST_FOREACH(const Osmium::OSM::Tag& tag, b->tags()) {
                    if (!ignore_tag(tag.key())) {
                        if (tag_map[tag.key()] != tag.value()) return false;
                        tag_map.erase(tag.key());
                    }
                }

                if (!tag_map.empty()) return false;

                return true;
            }

            /**
             * Check if the object is without tags ignoring tags with certain
             * keys defined in the ignore_tag() method.
             *
             * @returns true if this object has no tags, false otherwise
             */
            bool untagged(const Osmium::OSM::Object* object) const {
                if (object == NULL) return true;

                BOOST_FOREACH(const Osmium::OSM::Tag& tag, object->tags()) {
                    if (!ignore_tag(tag.key()) ) {
                        return false;
                    }
                }

                return true;
            }

            /**
             * Merge tags from way into the new area.
             *
             * @returns false if there was a collision, true otherwise
             */
            bool merge_tags(const Osmium::OSM::Way* way) {
                bool rv = true;

                std::map<std::string, std::string> tag_map;

                BOOST_FOREACH(const Osmium::OSM::Tag& tag, m_new_area->tags()) {
                    if (!ignore_tag(tag.key())) {
                        tag_map[tag.key()] = tag.value();
                    }
                }

                BOOST_FOREACH(const Osmium::OSM::Tag& tag, way->tags()) {
                    if (ignore_tag(tag.key())) continue;

                    if (tag_map.find(tag.key()) != tag_map.end()) {
                        if (tag_map[tag.key()] != tag.value()) rv = false;
                    } else {
                        m_new_area->tags().add(tag.key(), tag.value());
                        tag_map[tag.key()] = tag.value();
                    }
                }

                return rv;
            }

        public:

            Builder(const Osmium::Relations::RelationInfo& relation_info, bool attempt_repair) :
                m_relation_info(relation_info),
                m_areas(),
                m_attempt_repair(attempt_repair),
                m_new_area(make_shared<Osmium::OSM::Area>(*relation_info.relation())) {
            }

            /**
             * Build the area object(s) and return it/them.
             *
             * @returns All areas built.
             */
            std::vector< shared_ptr<Osmium::OSM::Area> >& build() {
                try {
                    build_multipolygon();
                    m_areas.push_back(m_new_area);
                } catch (BuildError& error) {
                    std::cerr << "Building multipolygon based on relation " << m_relation_info.relation()->id() << " failed: " << error.what() << "\n";
                }
                return m_areas;
            }

#ifdef OSMIUM_WITH_MULTIPOLYGON_PROFILING
            static std::vector<std::pair<std::string, timer*> > timers;

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
                timers.push_back(std::pair<std::string, timer*> ("   thereof assemble_ways", &assemble_ways_timer));
                timers.push_back(std::pair<std::string, timer*> ("   thereof make_one_ring", &make_one_ring_timer));
                timers.push_back(std::pair<std::string, timer*> ("      thereof union", &mor_union_timer));
                timers.push_back(std::pair<std::string, timer*> ("      thereof polygonizer", &mor_polygonizer_timer));
                timers.push_back(std::pair<std::string, timer*> ("   thereof contains", &contains_timer));
                timers.push_back(std::pair<std::string, timer*> ("   thereof extra_polygons", &extra_polygons_timer));
                timers.push_back(std::pair<std::string, timer*> ("   thereof polygon_build", &polygon_build_timer));
                timers.push_back(std::pair<std::string, timer*> ("      thereof inner_ring_touch", &inner_ring_touch_timer));
                timers.push_back(std::pair<std::string, timer*> ("      thereof intersections", &polygon_intersection_timer));
                timers.push_back(std::pair<std::string, timer*> ("   thereof multipolygon_build", &multipolygon_build_timer));
                timers.push_back(std::pair<std::string, timer*> ("   thereof multipolygon_write", &multipolygon_write_timer));
                timers.push_back(std::pair<std::string, timer*> ("   thereof error_write", &error_write_timer));
            }

            static void print_timings() {
                for (unsigned int i=0; i<timers.size(); ++i) {
                    std::cerr << timers[i].first << ": " << *(timers[i].second) << std::endl;
                }
            }
#endif // OSMIUM_WITH_MULTIPOLYGON_PROFILING

        private:

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
            geos::geom::LinearRing* create_non_intersecting_linear_ring(geos::geom::CoordinateSequence* orig_cs) const {
                const std::vector<geos::geom::Coordinate>* coords = orig_cs->toVector();
                int inv = coords->size();
                int val = 0;
                int current = (inv + val) / 2;
                bool simple;

                // find the longest non-intersecting stretch from the beginning
                // of the way.
                while (true) {
                    std::vector<geos::geom::Coordinate>* vv = new std::vector<geos::geom::Coordinate>(coords->begin(), coords->begin() + current);
                    geos::geom::CoordinateSequence* cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(vv);
                    geos::geom::LineString* a = Osmium::Geometry::geos_geometry_factory()->createLineString(cs);
                    if (!(simple = a->isSimple())) {
                        inv = current;
                    } else {
                        val = current;
                    }
                    delete a;
                    if (current == (inv+val)/2) break;
                    current = (inv + val) / 2;
                }

                if (!simple) --current;

                unsigned int cutoutstart = current;

                inv = 0;
                val = coords->size();
                current = (inv + val) / 2;

                // find the longest non-intersecting stretch from the end
                // of the way. Note that this is likely to overlap with the
                // stretch found above - assume a 10-node way where nodes 3
                // and 7 are identical, then we will find the sequence 0..6
                // above, and 4..9 here!

                while (true) {
                    std::vector<geos::geom::Coordinate>* vv = new std::vector<geos::geom::Coordinate>(coords->begin() + current, coords->end());
                    geos::geom::CoordinateSequence* cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(vv);
                    geos::geom::LineString* a = Osmium::Geometry::geos_geometry_factory()->createLineString(cs);
                    if (!(simple = a->isSimple())) {
                        inv = current;
                    } else {
                        val = current;
                    }
                    delete a;
                    if (current == (inv+val)/2) break;
                    current = (inv + val) / 2;
                }
                if (!simple) ++current;
                unsigned int cutoutend = current;

                // assemble a new linear ring by cutting out the problematic bit.
                // if the "problematic bit" however is longer than half the way,
                // then try using the "problematic bit" by itself.

                std::vector<geos::geom::Coordinate>* vv = new std::vector<geos::geom::Coordinate>();
                if (cutoutstart < cutoutend) {
                    unsigned int t = cutoutstart;
                    cutoutstart = cutoutend;
                    cutoutend = t;
                }
                if (cutoutstart-cutoutend > coords->size() / 2) {
                    vv->insert(vv->end(), coords->begin() + cutoutend, coords->begin() + cutoutstart);
                    vv->insert(vv->end(), vv->at(0));
                } else {
                    vv->insert(vv->end(), coords->begin(), coords->begin() + cutoutend);
                    vv->insert(vv->end(), coords->begin() + cutoutstart, coords->end());
                }
                geos::geom::CoordinateSequence* cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(vv);
                geos::geom::LinearRing* a = Osmium::Geometry::geos_geometry_factory()->createLinearRing(cs);

                // if this results in a valid ring, return it; else return NULL.

                if (!a->isValid()) return NULL;
                geos::geom::LinearRing* b = dynamic_cast<geos::geom::LinearRing*>(a->clone());
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
            RingInfo* make_one_ring(std::vector<WayInfo*>& ways, osm_object_id_t first, osm_object_id_t last, int ringcount, int sequence) const {

                // have we found a loop already?
                if (first && first == last) {
                    geos::geom::CoordinateSequence* cs = geos::geom::CoordinateArraySequenceFactory::instance()->create((size_t)0, (size_t)0);
                    geos::geom::LinearRing* lr = NULL;
                    try {
                        START_TIMER(mor_polygonizer);
                        WayInfo** sorted_ways = new WayInfo*[sequence];
                        for (unsigned int i=0; i<ways.size(); ++i) {
                            if (ways[i]->used == ringcount) {
                                sorted_ways[ways[i]->sequence] = ways[i];
                            }
                        }
                        for (int i=0; i<sequence; ++i) {
                            geos::geom::LineString* linestring = dynamic_cast<geos::geom::LineString*>(sorted_ways[i]->way_geom);
                            assert(linestring);
                            cs->add(linestring->getCoordinatesRO(), false, !sorted_ways[i]->invert);
                        }
                        delete[] sorted_ways;
                        lr = Osmium::Geometry::geos_geometry_factory()->createLinearRing(cs);
                        STOP_TIMER(mor_polygonizer);
                        if (!lr->isSimple() || !lr->isValid()) {
                            //delete lr;
                            lr = NULL;
                            if (m_attempt_repair) {
                                lr = create_non_intersecting_linear_ring(cs);
                                if (lr) {
                                    std::cerr << "Successfully repaired an invalid ring" << std::endl;
                                }
                            }
                            if (!lr) return NULL;
                        }
                        bool ccw = geos::algorithm::CGAlgorithms::isCCW(lr->getCoordinatesRO());
                        return new RingInfo(Osmium::Geometry::geos_geometry_factory()->createPolygon(lr, NULL), ccw ? COUNTERCLOCKWISE : CLOCKWISE);
                    } catch (const geos::util::GEOSException& exc) {
                        std::cerr << "Exception: " << exc.what() << std::endl;
                        return NULL;
                    }
                }

                // have we not allocated anything yet, then simply start with first available way,
                // or return NULL if all are taken.
                if (!first) {
                    for (unsigned int i=0; i<ways.size(); ++i) {
                        if (ways[i]->used != -1) continue;
                        ways[i]->used = ringcount;
                        ways[i]->sequence = 0;
                        ways[i]->invert = false;
                        RingInfo* rl = make_one_ring(ways, ways[i]->firstnode, ways[i]->lastnode, ringcount, 1);
                        if (rl) {
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

                for (unsigned int i=0; i<ways.size(); ++i) {
                    if (ways[i]->used < 0) ways[i]->tried = false;
                }

                for (unsigned int i=0; i<ways.size(); ++i) {
                    // ignore used ways
                    if (ways[i]->used >= 0) continue;
                    if (ways[i]->tried) continue;
                    ways[i]->tried = true;

                    int old_used = ways[i]->used;
                    if (ways[i]->firstnode == last) {
                        // add way to end
                        ways[i]->used = ringcount;
                        ways[i]->sequence = sequence;
                        ways[i]->invert = false;
                        RingInfo* result = make_one_ring(ways, first, ways[i]->lastnode, ringcount, sequence+1);
                        if (result) {
                            result->ways.push_back(ways[i]);
                            return result;
                        }
                        ways[i]->used = old_used;
                    } else if (ways[i]->lastnode == last) {
                        // add way to end, but turn it around
                        ways[i]->used = ringcount;
                        ways[i]->sequence = sequence;
                        ways[i]->invert = true;
                        RingInfo* result = make_one_ring(ways, first, ways[i]->firstnode, ringcount, sequence+1);
                        if (result) {
                            result->ways.push_back(ways[i]);
                            return result;
                        }
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
            bool find_and_repair_holes_in_rings(std::vector<WayInfo*>* ways) const {
                // collect the remaining debris (=unused ways) and find dangling nodes.

                std::map<int, geos::geom::Point*> dangling_node_map;
                for (std::vector<WayInfo*>::iterator i(ways->begin()); i != ways->end(); ++i) {
                    if ((*i)->used < 0) {
                        (*i)->innerouter = UNSET;
                        (*i)->used = -1;
                        for (int j=0; j<2; ++j) {
                            int nid = j ? (*i)->firstnode : (*i)->lastnode;
                            if (dangling_node_map[nid]) {
                                delete dangling_node_map[nid];
                                dangling_node_map[nid] = NULL;
                            } else {
                                dangling_node_map[nid] = j ? (*i)->get_firstnode_geom() : (*i)->get_lastnode_geom();
                            }
                        }
                    }
                }

                do {
                    int mindist_id = 0;
                    double mindist = -1;
                    int node1_id = 0;
                    geos::geom::Point* node1 = NULL;
                    geos::geom::Point* node2 = NULL;

                    // find one pair consisting of a random node from the list (node1)
                    // plus the node that lies closest to it.
                    for (std::map<int, geos::geom::Point*>::iterator i(dangling_node_map.begin()); i != dangling_node_map.end(); ++i) {
                        if (!i->second) continue;
                        if (node1 == NULL) {
                            node1 = i->second;
                            node1_id = i->first;
                            i->second = NULL;
                            mindist = -1;
                        } else {
# if GEOS_VERSION_MAJOR < 3 || (GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR <= 2)
                            double dist = geos::operation::distance::DistanceOp::distance(node1, (i->second)); // deprecated in newer version of GEOS
# else
                            double dist = geos::operation::distance::DistanceOp::distance(*node1, *(i->second));
# endif
                            if ((dist < mindist) || (mindist < 0)) {
                                mindist = dist;
                                mindist_id = i->first;
                            }
                        }
                    }

                    // if such a pair has been found, synthesize a connecting way.
                    if (node1 && mindist > -1) {
                        // if we find that there are dangling nodes but aren't
                        // repairing - break out.
                        if (!m_attempt_repair) return false;

                        // drop node2 from dangling map
                        node2 = dangling_node_map[mindist_id];
                        dangling_node_map[mindist_id] = NULL;

                        std::vector<geos::geom::Coordinate>* c = new std::vector<geos::geom::Coordinate>;
                        c->push_back(*(node1->getCoordinate()));
                        c->push_back(*(node2->getCoordinate()));
                        geos::geom::CoordinateSequence* cs = Osmium::Geometry::geos_geometry_factory()->getCoordinateSequenceFactory()->create(c);
                        geos::geom::Geometry* geometry = (geos::geom::Geometry*) Osmium::Geometry::geos_geometry_factory()->createLineString(cs);
                        ways->push_back(new WayInfo(geometry, node1_id, mindist_id, UNSET));
                        std::cerr << "fill gap between nodes " << node1_id << " and " << mindist_id << std::endl;
                    } else {
                        break;
                    }
                } while (1);

                return true;
            }

            /**
             * Assemble all ways which are members of this relation into a
             * vector of WayInfo elements. this holds room for the way pointer
             * and some extra flags.
             */
            void assemble_ways(std::vector<WayInfo*>& way_infos) {

                START_TIMER(assemble_ways);

                BOOST_FOREACH(const shared_ptr<Osmium::OSM::Object const>& object, m_relation_info.members()) {
                    const Osmium::OSM::Way* way = dynamic_cast<const Osmium::OSM::Way*>(object.get());

                    if (way->timestamp() > m_new_area->timestamp()) {
                        m_new_area->timestamp(way->timestamp());
                    }

                    WayInfo* wi = new WayInfo(way, UNSET);
                    if (!wi->way_geom) {
                        delete wi;
                        throw InvalidWayGeometry("Invalid way geometry in multipolygon relation member");
                    }
                    way_infos.push_back(wi);
                    // TODO drop duplicate ways automatically in repair mode?
                    // TODO maybe add INNER/OUTER instead of UNSET to enable later warnings on role mismatch
                }

                STOP_TIMER(assemble_ways);
            }

                // convenience defines to aid in clearing up on error return.
#define clear_ringlist() \
                for (std::vector<RingInfo*>::const_iterator rli(ringlist.begin()); rli != ringlist.end(); ++rli) delete *rli;
#define clear_wayinfo() \
                for (std::vector<WayInfo*>::const_iterator win(ways.begin()); win != ways.end(); ++win) delete *win;


            void make_rings(std::vector<RingInfo*>& ringlist, std::vector<WayInfo*>& ways) const {
                // try and create as many closed rings as possible from the assortment
                // of ways. make_one_ring will automatically flag those that have been
                // used so they are not used again.
                do {
                    START_TIMER(make_one_ring);
                    RingInfo* r = make_one_ring(ways, 0, 0, ringlist.size(), 0);
                    STOP_TIMER(make_one_ring);
                    if (r == NULL) break;
                    ringlist.push_back(r);
                } while (true);

                if (ringlist.empty()) {
                    // FIXME throw NoRings("no rings");
                }

                if (!find_and_repair_holes_in_rings(&ways)) {
                    clear_ringlist();
                    clear_wayinfo();
                    throw DanglingEnds("un-connectable dangling ends");
                }

                // re-run ring building, taking into account the newly created "repair" bits.
                // (in case there were no dangling bits, make_one_ring terminates quickly.)
                do {
                    START_TIMER(make_one_ring);
                    RingInfo* r = make_one_ring(ways, 0, 0, ringlist.size(), 0);
                    STOP_TIMER(make_one_ring);
                    if (r == NULL) break;
                    ringlist.push_back(r);
                } while (true);

                if (ringlist.empty()) {
                    clear_ringlist();
                    clear_wayinfo();
                    throw NoRings("no rings");
                }
            }

            /**
             * Find out which ring contains which other ring, so we know
             * which are inner rings and which outer. Don't trust the "role"
             * specifications.
             */
            void determine_inner_outer_rings(std::vector<RingInfo*>& ringlist) const {
                START_TIMER(contains);

                typedef boost::dynamic_bitset<> bs_t;

                std::vector<bs_t> contains(ringlist.size(), bs_t(ringlist.size()));

                bs_t contained_by_even_number(ringlist.size());
                contained_by_even_number.set();

                // build contains relationships.
                // we use contained_by_even_number as a helper for us to determine
                // whether something is an inner (false) or outer (true) ring.

                for (unsigned int i=0; i<ringlist.size(); ++i) {
                    const boost::scoped_ptr<geos::geom::prep::PreparedPolygon> pp(new geos::geom::prep::PreparedPolygon(ringlist[i]->polygon));
                    for (unsigned int j=0; j<ringlist.size(); ++j) {
                        if (i==j) continue;
                        if (contains[j][i]) continue;
                        contains[i][j] = pp->contains(ringlist[j]->polygon);
                        contained_by_even_number[j] ^= contains[i][j];
                    }
                }

                // we now have an array that has a true value whenever something is
                // contained by something else; if a contains b and b contains c, then
                // our array says that a contains b, b contains c, and a contains c.
                // thin out the array so that only direct relationships remain (and
                // the "a contains c" is dropped).

                for (unsigned int i=0; i<ringlist.size(); ++i) {
                    for (unsigned j=0; j<ringlist.size(); ++j) {
                        if (contains[i][j]) {
                            // see if there is an intermediary relationship
                            for (unsigned int k=0; k<ringlist.size(); ++k) {
                                if (k==i) continue;
                                if (k==j) continue;
                                if (contains[i][k] && contains[k][j]) {
                                    // intermediary relationship exists; break this
                                    // one up.
                                    contains[i][j] = false;
                                    break;
                                }
                            }
                        }
                    }
                }

                // populate the "inner_rings" list and the "contained_by" pointer
                // in the ring list based on the data collected. the "contains"
                // array can be thrown away afterwards.

                for (unsigned int i=0; i<ringlist.size(); ++i) {
                    for (unsigned int j=0; j<ringlist.size(); ++j) {
                        if (contains[i][j] && !contained_by_even_number[j]) {
                            ringlist[j]->contained_by = ringlist[i];
                            ringlist[i]->inner_rings.push_back(ringlist[j]);
                        }
                    }
                }

                STOP_TIMER(contains);
            }

            /**
             * now look at all enclosed (inner) rings that consist of only one way.
             * if such an inner ring has way tags, do the following:
             * - emit an extra polygon for the inner ring if the tags are different
             *   from the relation's
             * - emit a warning, and ignore the inner ring, if the tags are the same
             *   as for the relation
             **/
            int handle_one_way_inner_rings(std::vector<RingInfo*>& ringlist) {
                START_TIMER(extra_polygons);
                int outer_ring_count = 0;
                for (unsigned int i=0; i<ringlist.size(); ++i) {
                    if (ringlist[i]->contained_by) {
                        if (ringlist[i]->ways.size() == 1 && !untagged(ringlist[i]->ways[0]->way)) {
                            std::vector<geos::geom::Geometry*>* g = new std::vector<geos::geom::Geometry*>;
                            if (ringlist[i]->direction == CLOCKWISE) {
                                g->push_back(ringlist[i]->polygon->clone());
                            } else {
                                geos::geom::LineString* tmp = dynamic_cast<geos::geom::LineString*>(ringlist[i]->polygon->getExteriorRing()->reverse());
                                assert(tmp);
                                geos::geom::LinearRing* reversed_ring = Osmium::Geometry::geos_geometry_factory()->createLinearRing(tmp->getCoordinates());
                                delete tmp;
                                g->push_back(Osmium::Geometry::geos_geometry_factory()->createPolygon(reversed_ring, NULL));
                            }

                            geos::geom::MultiPolygon* special_mp = Osmium::Geometry::geos_geometry_factory()->createMultiPolygon(g);

                            if (same_tags(ringlist[i]->ways[0]->way, m_new_area.get())) {
                                // warning
                                // warnings.insert("duplicate_tags_on_inner");
                            } else if (ringlist[i]->contained_by->ways.size() == 1 && same_tags(ringlist[i]->ways[0]->way, ringlist[i]->contained_by->ways[0]->way)) {
                                // warning
                                // warnings.insert("duplicate_tags_on_inner");
                            } else {
                                shared_ptr<Osmium::OSM::Area> internal_area = make_shared<Osmium::OSM::Area>(*(ringlist[i]->ways[0]->way));
                                internal_area->geos_geometry(special_mp);
                                m_areas.push_back(internal_area);

                                // Area destructor deletes the
                                // geometry, so avoid to delete it again.
                                special_mp = NULL;
                            }
                            delete special_mp;
                        }
                    } else {
                        ++outer_ring_count;
                    }
                }
                STOP_TIMER(extra_polygons);

                return outer_ring_count;
            }

            /**
            * Tries to build a multipolygon.
            */
            void build_multipolygon() {
                std::vector<WayInfo*> ways;

                assemble_ways(ways);

                std::vector<RingInfo*> ringlist;

                make_rings(ringlist, ways);

                determine_inner_outer_rings(ringlist);

                int outer_ring_count = handle_one_way_inner_rings(ringlist);

                // for all non-enclosed rings, assemble holes and build polygon.

                std::vector<geos::geom::Geometry*>* polygons = new std::vector<geos::geom::Geometry*>();

                START_TIMER(polygon_build)
                for (unsigned int i=0; i<ringlist.size(); ++i) {
                    // look only at outer, i.e. non-contained rings. each ends up as one polygon.
                    if (ringlist[i] == NULL) continue; // can happen if ring has been deleted
                    if (ringlist[i]->contained_by) continue;

                    std::vector<geos::geom::Geometry*>* holes = new std::vector<geos::geom::Geometry*>(); // ownership is later transferred to polygon

                    START_TIMER(inner_ring_touch)
                    for (int j=0; j<((int)ringlist[i]->inner_rings.size()-1); ++j) {
                        if (!ringlist[i]->inner_rings[j]->polygon) continue;
                        geos::geom::LinearRing* ring = (geos::geom::LinearRing*) ringlist[i]->inner_rings[j]->polygon->getExteriorRing();

                        // check if some of the rings touch another ring.

                        for (unsigned int k=j + 1; k<ringlist[i]->inner_rings.size(); ++k) {
                            if (!ringlist[i]->inner_rings[k]->polygon) continue;
                            const geos::geom::Geometry* compare = ringlist[i]->inner_rings[k]->polygon->getExteriorRing();
                            geos::geom::Geometry* inter = NULL;
                            try {
                                if (!ring->intersects(compare)) continue;
                                inter = ring->intersection(compare);
                            } catch (const geos::util::GEOSException& exc) {
                                // nop;
                            }
                            if (inter && (inter->getGeometryTypeId() == geos::geom::GEOS_LINESTRING || inter->getGeometryTypeId() == geos::geom::GEOS_MULTILINESTRING)) {
                                // touching inner rings
                                // this is allowed, but we must fix them up into a valid
                                // geometry
                                geos::geom::Geometry* diff = ring->symDifference(compare);
                                geos::operation::polygonize::Polygonizer* p = new geos::operation::polygonize::Polygonizer();
                                p->add(diff);
                                std::vector<geos::geom::Polygon*>* polys = p->getPolygons();
                                if (polys && polys->size() == 1) {
                                    ringlist[i]->inner_rings[j]->polygon = polys->at(0);
                                    bool ccw = geos::algorithm::CGAlgorithms::isCCW(polys->at(0)->getExteriorRing()->getCoordinatesRO());
                                    ringlist[i]->inner_rings[j]->direction = ccw ? COUNTERCLOCKWISE : CLOCKWISE;
                                    ringlist[i]->inner_rings[k]->polygon = NULL;
                                    j = -1;
                                    break;
                                }
                            } else {
                                // other kind of intersect between inner rings; this is
                                // not allwoed and will lead to an exception later when
                                // building the MP
                            }
                        }
                    }
                    STOP_TIMER(inner_ring_touch)

                    for (unsigned int j=0; j<ringlist[i]->inner_rings.size(); ++j) {
                        if (!ringlist[i]->inner_rings[j]->polygon) continue;
                        geos::geom::LinearRing* ring = (geos::geom::LinearRing*) ringlist[i]->inner_rings[j]->polygon->getExteriorRing(); // XXX cast is not const-correct!

                        if (ringlist[i]->inner_rings[j]->direction == CLOCKWISE) {
                            // reverse ring
                            geos::geom::LineString* tmp = dynamic_cast<geos::geom::LineString*>(ring->reverse());
                            assert(tmp);
                            geos::geom::LinearRing* reversed_ring = Osmium::Geometry::geos_geometry_factory()->createLinearRing(tmp->getCoordinates());
                            delete tmp;
                            holes->push_back(reversed_ring);
                        } else {
                            holes->push_back(ring);
                        }
                    }

                    geos::geom::LinearRing* ring = (geos::geom::LinearRing*)ringlist[i]->polygon->getExteriorRing(); // XXX cast is not const-correct!
                    if (ringlist[i]->direction == COUNTERCLOCKWISE) {
                        geos::geom::LineString* tmp = dynamic_cast<geos::geom::LineString*>(ring->reverse());
                        assert(tmp);
                        geos::geom::LinearRing* reversed_ring = Osmium::Geometry::geos_geometry_factory()->createLinearRing(tmp->getCoordinates());
                        ring = reversed_ring;
                        delete tmp;
                    } else {
                        ring = dynamic_cast<geos::geom::LinearRing*>(ring->clone());
                    }
                    delete ringlist[i]->polygon;
                    ringlist[i]->polygon = NULL;
                    geos::geom::Polygon* p = NULL;
                    bool valid = false;

                    try {
                        p = Osmium::Geometry::geos_geometry_factory()->createPolygon(ring, holes);
                        if (p) valid = p->isValid();
                    } catch (const geos::util::GEOSException& exc) {
                        // nop
                        std::cerr << "Exception during creation of polygon for relation #" << m_relation_info.relation()->id() << ": " << exc.what() << " (treating as invalid polygon)" << std::endl;
                    }
                    if (!valid) {
                        // polygon is invalid.
                        clear_ringlist();
                        clear_wayinfo();
                        if (p) delete p;
                        else delete ring;
                        throw InvalidRing("invalid ring");
                    } else {
                        polygons->push_back(p);
                        for (unsigned int k=0; k<ringlist[i]->ways.size(); ++k) {
                            WayInfo* wi = ringlist[i]->ways[k];
                            // may have "hole filler" ways in there, not backed by
                            // proper way and thus no tags:
                            if (wi->way == NULL) continue;
                            if (untagged(wi->way)) {
                                // way not tagged - ok
                            } else if (same_tags(m_new_area.get(), wi->way)) {
                                // way tagged the same as relation/previous ways, ok
                            } else if (untagged(m_new_area.get())) {
                                // relation untagged; use tags from way; ok
                                merge_tags(wi->way);
                            } else {
                                // this is grey-area terrain in OSM - we have tags on
                                // the relation and a different set of tags on the outer
                                // way(s). Use tags from outer ring only if there is
                                // only one outer ring and it has only one way.
                                if (outer_ring_count == 1 && ringlist[i]->ways.size() == 1) {
                                    merge_tags(wi->way);
                                }
                            }

                            wi->innerouter = OUTER;
                            if (wi->orig_innerouter == INNER) {
                                // warning: inner/outer mismatch
                            }
                        }
                    }
                    // later delete ringlist[i];
                    // ringlist[i] = NULL;
                }
                STOP_TIMER(polygon_build);

                clear_ringlist();
                clear_wayinfo();
                if (polygons->empty()) {
                    throw NoRings("no rings");
                }

                START_TIMER(multipolygon_build);
                bool valid = false;
                geos::geom::MultiPolygon* mp = NULL;
                try {
                    mp = Osmium::Geometry::geos_geometry_factory()->createMultiPolygon(polygons);
                    valid = mp->isValid();
                } catch (const geos::util::GEOSException& exc) {
                    // nop
                };
                STOP_TIMER(multipolygon_build);
                if (valid) {
                    m_new_area->geos_geometry(mp);
                    return;
                }
                throw InvalidMultiPolygon("multipolygon invalid");
            }

        }; // class Builder

    } // namespace MultiPolygon

} // namespace Osmium

#endif // OSMIUM_MULTIPOLYGON_BUILDER_HPP
