#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>
#include <sstream>
#include <iomanip>

#ifdef WITH_MULTIPOLYGON_PROFILING
#define START_TIMER(x) x##_timer.start();
#define STOP_TIMER(x) x##_timer.stop();
#else
#define START_TIMER(x)
#define STOP_TIMER(x)
#endif

#include "osmium.hpp"

#include <geos/geom/PrecisionModel.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/MultiLineString.h>
#include <geos/util/GEOSException.h>
#include <geos/opLinemerge.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/opPolygonize.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geomgraph/GeometryGraph.h>
#include <geos/geomgraph/index/SegmentIntersector.h>

namespace Osmium {

    namespace OSM {

#ifdef WITH_MULTIPOLYGON_PROFILING
        std::vector<std::pair<std::string, timer *> > MultipolygonFromRelation::timers;
#endif

        bool ignore_tag(const std::string &s) {
            if (s=="type") return true;
            if (s=="created_by") return true;
            if (s=="source") return true;
            if (s=="note") return true;
            return false;
        }

        bool same_tags(const Object *a, const Object *b) {
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
            if (r->tag_count() == 0) return true;
            for (int i=0; i < r->tag_count(); i++) {
                if (! ignore_tag(r->get_tag_key(i)) ) {
                    return false;
                }
            }
            return true;
        }


        /**
        * Tries to collect 1...n ways from the n ways in the given list so that
        * they form a closed ring. If this is possible, flag those as being used
        * by ring #ringcount in the way list and return the geometry. (The method
        * may be called again to find further rings.) If this is not possible, 
        * return NULL.
        */
        RingInfo *MultipolygonFromRelation::make_one_ring(std::vector<WayInfo> &ways, osm_object_id_t first, osm_object_id_t last, int ringcount, int sequence)
        {

            // have we found a loop already?
            if (first && first == last)
            {
                geos::geom::CoordinateSequence *cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(0, 0);
                geos::geom::LinearRing *lr = NULL;
                try
                {
                    START_TIMER(mor_polygonizer);
                    WayInfo *sorted_ways = new WayInfo[sequence];
                    for (unsigned int i=0; i<ways.size(); i++)
                    {
                        if (ways[i].used == ringcount) 
                        {
                            sorted_ways[ways[i].sequence] = ways[i];
                        }
                    }
                    for (int i=0; i<sequence; i++)
                    {
                        // cout << "seq " << i << ": add way " << sorted_ways[i].way->id << " from " << sorted_ways[i].way->firstnode << " to " << sorted_ways[i].way->lastnode;
                        // if (sorted_ways[i].invert) cout << " (INV)";
                        // cout << endl;
                        // cout << " this way:" << *(sorted_ways[i].way->geometry->getCoordinates()) << endl;
                        cs->add(((geos::geom::LineString *)sorted_ways[i].way->get_geometry())->getCoordinatesRO(), false, !sorted_ways[i].invert);
                        // cout << " new sequence:" << *cs << endl;
                    }
                    delete[] sorted_ways;
                    lr = Osmium::OSM::Object::global_geometry_factory->createLinearRing(cs);
                    STOP_TIMER(mor_polygonizer);
                    if (!lr->isSimple() || !lr->isValid())
                    { 
                        delete lr;
                        // delete cs;
                        return NULL; 
                    }
                    bool ccw = geos::algorithm::CGAlgorithms::isCCW(lr->getCoordinatesRO());
                    // cout << "direction: " << (ccw ? "counter" : "") << "clockwise" << endl;
                    RingInfo *rl = new RingInfo();
                    rl->direction = ccw ? COUNTERCLOCKWISE : CLOCKWISE;
                    rl->polygon = Osmium::OSM::Object::global_geometry_factory->createPolygon(lr, NULL);
                    return rl;
                }
                catch (const geos::util::GEOSException& exc) 
                {
                    // fprintf(stderr, "1 continuing after exception: %s\n", exc.what());
                    // if (lr) delete lr; else delete cs;
                    return NULL;
                }
            }

            // have we not allocated anything yet, then simply start with first available way, 
            // or return NULL if all are taken.
            if (!first)
            {
                for (unsigned int i=0; i<ways.size(); i++)
                {
                    if (ways[i].used != -1) continue;
                    ways[i].used = ringcount;
                    ways[i].sequence = 0;
                    ways[i].invert = false;
                    RingInfo *rl = make_one_ring(ways, ways[i].way->get_first_node_id(), ways[i].way->get_last_node_id(), ringcount, 1);
                    if (rl)
                    {
                        rl->ways.push_back(&(ways[i]));
                        return rl;
                    }
                    ways[i].used = -2;
                    break;
                }
                return NULL;
            }

            // try extending our current line at the rear end
            // since we are looking for a LOOP, no sense to try extending it at both ends 
            // as we'll eventually get there anyway!

            for (unsigned int i=0; i<ways.size(); i++)
            {
                if (ways[i].used < 0) ways[i].way->tried = false;
            }

            for (unsigned int i=0; i<ways.size(); i++)
            {
                // ignore used ways
                if (ways[i].used >= 0) continue;
                if (ways[i].way->tried) continue;
                ways[i].way->tried = true;

                Way *way = ways[i].way;

                int old_used = ways[i].used;
                if (way->get_first_node_id() == last)
                {
                    // add way to end
                    ways[i].used = ringcount;
                    ways[i].sequence = sequence;
                    ways[i].invert = false;
                    RingInfo *result = NULL;
                    result = make_one_ring(ways, first, way->get_last_node_id(), ringcount, sequence+1);
                    if (result) { result->ways.push_back(&(ways[i])); return result; }
                    ways[i].used = old_used;
                }
                else if (way->get_last_node_id() == last)
                {
                    // add way to end, but turn it around
                    ways[i].used = ringcount;
                    ways[i].sequence = sequence;
                    ways[i].invert = true;
                    RingInfo *result = NULL;
                    result = make_one_ring(ways, first, way->get_first_node_id(), ringcount, sequence+1);
                    if (result) { result->ways.push_back(&(ways[i])); return result; }
                    ways[i].used = old_used;
                }
            }
            return NULL;
        }


        /**
        * Tries to build a multipolygon from the given relation and writes the 
        * result to the appropriate output file(s).
        *
        */
        bool MultipolygonFromRelation::build_geometry()
        {
            std::vector<WayInfo> ways;

            // the timestamp of the multipolygon will be the maximum of the timestamp from the relation and from all member ways
            time_t timestamp = relation->get_timestamp();

            // assemble all ways which are members of this relation into a 
            // vector of WayInfo elements. this holds room for the way pointer
            // and some extra flags.
            
            START_TIMER(assemble_ways);
            for (std::vector<Way>::iterator i = member_ways.begin(); i != member_ways.end(); i++)
            {       
                if (i->get_timestamp() > timestamp) timestamp = i->get_timestamp();
                ways.push_back(WayInfo(&(*i), UNSET));
                // TODO drop duplicate ways automatically
            // TODO: statt UNSET sollte hier INNER/OUTER je nach role, wird aber nur fuer warnings gebraucht
            }
            STOP_TIMER(assemble_ways);

            std::vector<RingInfo *> ringlist;

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
            } while(1);

            if (ringlist.empty())
            {
                return geometry_error("no rings");
            }

            // collect the remaining debris.
            
            std::map<int,bool> dangling_node;

            std::vector<geos::geom::Geometry *> v;
                        
            for (std::vector<WayInfo>::iterator i = ways.begin(); i != ways.end(); i++)
            {       
                if (i->used < 0)
                {
                    v.push_back(i->way->get_geometry());
                    i->innerouter = UNSET;
                    dangling_node[i->way->get_first_node_id()] = !dangling_node[i->way->get_first_node_id()];
                    dangling_node[i->way->get_last_node_id()] = !dangling_node[i->way->get_last_node_id()];
                }
            }

            if (v.size())
            {
                // FIXME repair errors
                return geometry_error("unconnected ways");
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
                            Osmium::OSM::Object::global_geometry_factory->createLinearRing(tmp->getCoordinates());
                            delete tmp;
                            g->push_back(Osmium::OSM::Object::global_geometry_factory->createPolygon(reversed_ring, NULL));
                        }

                        geos::geom::MultiPolygon *special_mp = Osmium::OSM::Object::global_geometry_factory->createMultiPolygon(g);

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
                            std::cerr << " XXX internal mp\n";
                            Osmium::OSM::Way *first_way = ringlist[i]->ways[0]->way; // to simplify things we get some metadata only from the first way in this ring
                            Osmium::OSM::MultipolygonFromRelation *internal_mp = new Osmium::OSM::MultipolygonFromRelation(relation, boundary, special_mp, first_way->tags, first_way->get_timestamp());
                            callback(internal_mp);
                            /* emit polygon
                            fprintf(complexFile, "%s\t%s\tM\t%.2f\t%s",
                                "\\N", // do not output r->id since this is not a one-on-one matching
                                ringlist[i]->ways[0]->way->timestamp.c_str(),
                                ringlist[i]->polygon->getArea(),
                                geomAsHex(special_mp).c_str());
                            ringlist[i]->ways[0]->way->writeTags(complexFile);
                            fprintf(complexFile, "\n");
                            */
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
                            Osmium::OSM::Object::global_geometry_factory->createLinearRing(tmp->getCoordinates());
                        delete tmp;
                        holes->push_back(reversed_ring);
                    }
                    else
                    {
                        holes->push_back(ring);
                    }

                    for (unsigned int k=0; k<ringlist[i]->inner_rings[j]->ways.size(); k++)
                    {       
                        WayInfo *wi = ringlist[i]->inner_rings[j]->ways[k];
                        wi->innerouter = INNER;
                        if (ringlist[i]->inner_rings[j]->direction == CLOCKWISE) 
                        {
                            wi->invert = !wi->invert;
                        }
                        if (wi->orig_innerouter == OUTER)
                        { 
                            // warning: inner/outer mismatch
                        }
                        wi->poly_id = polygons->size();
                    }
                    //delete ringlist[i]->inner_rings[j];
                    //ringlist[i]->inner_rings[j] = NULL;
                }

                geos::geom::LinearRing *ring = (geos::geom::LinearRing *) ringlist[i]->polygon->getExteriorRing();
                if (ringlist[i]->direction == COUNTERCLOCKWISE)
                {
                    geos::geom::LineString *tmp = (geos::geom::LineString *) ring->reverse();
                    geos::geom::LinearRing *reversed_ring = 
                        Osmium::OSM::Object::global_geometry_factory->createLinearRing(tmp->getCoordinates());
                    ring = reversed_ring;
                    for (unsigned int k=0; k<ringlist[i]->ways.size(); k++) 
                    {
                        // cout << "way " << ringlist[i]->ways[k]->way->id << " is part of a CCW outer ring, invert" << endl;
                        ringlist[i]->ways[k]->invert = !ringlist[i]->ways[k]->invert;
                    }
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
                    p = Osmium::OSM::Object::global_geometry_factory->createPolygon(ring, holes);
                    if (p) valid = p->isValid();
                }
                catch (const geos::util::GEOSException& exc) 
                {
                    // nop
                    std::cerr << "Exception during creation of polygon for relation #" << relation->id << ": " << exc.what() << " (treating as invalid polygon)" << std::endl;
                }
                if (!valid)
                {
                    // polygon is invalid.
                    return geometry_error("invalid ring");
                }
                else
                {
                    polygons->push_back(p);
                    for (unsigned int k=0; k<ringlist[i]->ways.size(); k++)
                    {       
                        WayInfo *wi = ringlist[i]->ways[k];
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
                        /**
                        MERGING TAGS IS DISABLED

                        else if (merge_tags(r, wi->way))
                        {
                            // tags merged
                            // wi->errorhint = "tags_merged_with_relation";
                            // tag_warning = true;
                        }
                        else
                        {
                            // tags in conflict
                            // wi->errorhint = "tag_conflict_with_relation";
                            // errmsg = "tag_conflict";
                        }
                        */
                        wi->innerouter = OUTER;
                        if (wi->orig_innerouter == INNER && wi->errorhint.empty()) 
                        { 
                            // waring: inner/outer mismatch
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

            if (polygons->empty()) return geometry_error("no rings");

            START_TIMER(multipolygon_build);
            bool valid = false;
            try
            {
                mp = Osmium::OSM::Object::global_geometry_factory->createMultiPolygon(polygons);
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

        bool MultipolygonFromRelation::geometry_error(const char *message)
        {
            geometry_error_message = message;
            std::cerr << "building mp failed: " << geometry_error_message << std::endl;
            geometry = NULL;
            return false;
        }

    } // namespace OSM

} // namespace Osmium

