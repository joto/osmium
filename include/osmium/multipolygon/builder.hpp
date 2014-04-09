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

#include <cassert>
#include <map>
#include <stdexcept>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/dynamic_bitset.hpp>

#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/util/GEOSException.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/algorithm/CGAlgorithms.h>

#include <osmium/smart_ptr.hpp>
#include <osmium/osm.hpp>
#include <osmium/geometry.hpp>
#include <osmium/geometry/geos.hpp>
#include <osmium/geometry/haversine.hpp>
#include <osmium/relations/relation_info.hpp>

namespace Osmium {

    namespace MultiPolygon {

        struct BuildError : public std::runtime_error {
            BuildError(const std::string& what) :
                std::runtime_error(what) {
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

        public:

            const shared_ptr<Osmium::OSM::Way const> way;
            int used;
            int sequence;
            bool invert;
            innerouter_t innerouter;

            WayInfo(const shared_ptr<Osmium::OSM::Way const>& w) :
                way(w),
                used(-1),
                sequence(0),
                invert(false),
                innerouter(UNSET) {
            }

            osm_object_id_t firstnode() const {
                return way->nodes().front().ref();
            }

            osm_object_id_t lastnode() const {
                return way->nodes().back().ref();
            }

        private:

            // objects of this class can't be copied
            WayInfo(const WayInfo&);
            WayInfo& operator=(const WayInfo&);

        }; // class WayInfo

        class RingInfo {

        public:

            geos::geom::Polygon* polygon;
            direction_t direction;
            std::vector< shared_ptr<WayInfo> > ways;
            std::vector< shared_ptr<RingInfo> > inner_rings;
            weak_ptr<RingInfo> contained_by;

            RingInfo(geos::geom::Polygon* p, direction_t dir) :
                polygon(p),
                direction(dir),
                ways(),
                inner_rings(),
                contained_by() {
            }

            ~RingInfo() {
                delete polygon;
            }

            /// Is this an inner ring?
            bool is_inner() const {
                return !contained_by.expired();
            }

            /**
             * Return a copy of the outer ring of the polygon geometry in the
             * given direction.
             *
             * Caller takes ownership.
             */
            geos::geom::LinearRing* ring_in_direction(direction_t dir) const {
                if (direction == dir) {
                    return dynamic_cast<geos::geom::LinearRing*>(polygon->getExteriorRing()->clone());
                } else {
                    geos::geom::LineString* tmp = dynamic_cast<geos::geom::LineString*>(polygon->getExteriorRing()->reverse());
                    assert(tmp);
                    geos::geom::LinearRing* reversed_ring = Osmium::Geometry::geos_geometry_factory()->createLinearRing(tmp->getCoordinates());
                    delete tmp;
                    return reversed_ring;
                }
            }

        private:

            // objects of this class can't be copied
            RingInfo(const RingInfo&);
            RingInfo& operator=(const RingInfo&);

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

            std::vector< shared_ptr<RingInfo> > m_ringlist;

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
                    if (!ignore_tag(tag.key())) {
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

            // objects of this class can't be copied
            Builder(const Builder&);
            Builder& operator=(const Builder&);

        public:

            Builder(const Osmium::Relations::RelationInfo& relation_info, bool attempt_repair) :
                m_relation_info(relation_info),
                m_areas(),
                m_attempt_repair(attempt_repair),
                m_new_area(make_shared<Osmium::OSM::Area>(*relation_info.relation())),
                m_ringlist() {
            }

            /**
             * Build the area object(s) and return it/them.
             *
             * @returns All areas built. This is a reference to a vector
             *          internal to the Builder object. Do not use it after
             *          the Builder object is gone.
             */
            std::vector< shared_ptr<Osmium::OSM::Area> >& build() {
                try {
                    {
                        std::vector< shared_ptr<WayInfo> > ways;
                        assemble_ways(ways);
                        make_rings(ways);
                    }

                    determine_inner_outer_rings();

                    build_multipolygon();
                    m_areas.push_back(m_new_area);
                } catch (BuildError& error) {
                    std::cerr << "Building multipolygon based on relation " << m_relation_info.relation()->id() << " failed: " << error.what() << "\n";
                }
                return m_areas;
            }

        private:

            /**
            * This helper gets called when we find a ring that is not valid -
            * usually because it self-intersects. The method tries to salvage
            * as much of the ring as possible, using binary search to find the
            * bit that needs to be cut out.
            *
            * @returns A valid LinearRing, or NULL if none can be built.
            *
            * Caller takes ownership.
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
                    std::vector<geos::geom::Coordinate>* vcoords = new std::vector<geos::geom::Coordinate>(coords->begin(), coords->begin() + current);
                    geos::geom::CoordinateSequence* coordinate_sequence = geos::geom::CoordinateArraySequenceFactory::instance()->create(vcoords);
                    geos::geom::LineString* linestring = Osmium::Geometry::geos_geometry_factory()->createLineString(coordinate_sequence);
                    if (!(simple = linestring->isSimple())) {
                        inv = current;
                    } else {
                        val = current;
                    }
                    delete linestring;
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
                    std::vector<geos::geom::Coordinate>* vcoords = new std::vector<geos::geom::Coordinate>(coords->begin() + current, coords->end());
                    geos::geom::CoordinateSequence* coordinate_sequence = geos::geom::CoordinateArraySequenceFactory::instance()->create(vcoords);
                    geos::geom::LineString* linestring = Osmium::Geometry::geos_geometry_factory()->createLineString(coordinate_sequence);
                    if (!(simple = linestring->isSimple())) {
                        inv = current;
                    } else {
                        val = current;
                    }
                    delete linestring;
                    if (current == (inv+val)/2) break;
                    current = (inv + val) / 2;
                }

                if (!simple) ++current;

                unsigned int cutoutend = current;

                // assemble a new linear ring by cutting out the problematic bit.
                // if the "problematic bit" however is longer than half the way,
                // then try using the "problematic bit" by itself.

                if (cutoutstart < cutoutend) {
                    std::swap(cutoutstart, cutoutend);
                }

                std::vector<geos::geom::Coordinate>* vv = new std::vector<geos::geom::Coordinate>();
                if (cutoutstart-cutoutend > coords->size() / 2) {
                    vv->insert(vv->end(), coords->begin() + cutoutend, coords->begin() + cutoutstart);
                    vv->insert(vv->end(), vv->at(0));
                } else {
                    vv->insert(vv->end(), coords->begin(), coords->begin() + cutoutend);
                    vv->insert(vv->end(), coords->begin() + cutoutstart, coords->end());
                }
                geos::geom::CoordinateSequence* cs = geos::geom::CoordinateArraySequenceFactory::instance()->create(vv);
                geos::geom::LinearRing* linear_ring = Osmium::Geometry::geos_geometry_factory()->createLinearRing(cs);

                if (linear_ring->isValid()) {
                    return linear_ring;
                }

                delete linear_ring;
                return NULL;
            }

            /**
             * Create CoordinateSequence from Ways in a vector of WayInfos.
             *
             * Caller takes ownership.
             */
            geos::geom::CoordinateSequence* create_ring_coordinate_sequence(std::vector< shared_ptr<WayInfo> >& ways) const {
                geos::geom::CoordinateSequence* coordinates = Osmium::Geometry::geos_geometry_factory()->getCoordinateSequenceFactory()->create(static_cast<size_t>(0), 2);

                BOOST_FOREACH(const shared_ptr<WayInfo>& way_info, ways) {
                    if (way_info->invert) {
                        BOOST_REVERSE_FOREACH(const Osmium::OSM::WayNode& wn, way_info->way->nodes()) {
                            coordinates->add(Osmium::Geometry::create_geos_coordinate(wn.position()), false);
                        }
                    } else {
                        BOOST_FOREACH(const Osmium::OSM::WayNode& wn, way_info->way->nodes()) {
                            coordinates->add(Osmium::Geometry::create_geos_coordinate(wn.position()), false);
                        }
                    }
                }

                return coordinates;
            }

            /**
             * This method is called when a complete ring was created from one or more ways.
             */
            shared_ptr<RingInfo> ring_is_complete(std::vector< shared_ptr<WayInfo> >& ways, int ringcount, int num_ways_in_ring) const {
                std::vector< shared_ptr<WayInfo> > sorted_ways(num_ways_in_ring);
                for (unsigned int i=0; i < ways.size(); ++i) {
                    if (ways[i]->used == ringcount) {
                        sorted_ways[ways[i]->sequence] = ways[i];
                    }
                }

                try {
                    geos::geom::LinearRing* linear_ring = Osmium::Geometry::geos_geometry_factory()->createLinearRing(create_ring_coordinate_sequence(sorted_ways));

                    if (!linear_ring->isSimple() || !linear_ring->isValid()) {
                        delete linear_ring;
                        linear_ring = NULL;
                        if (m_attempt_repair) {
                            scoped_ptr<geos::geom::CoordinateSequence> cs(create_ring_coordinate_sequence(sorted_ways));
                            linear_ring = create_non_intersecting_linear_ring(cs.get());
                            if (linear_ring) {
                                std::cerr << "Successfully repaired an invalid ring" << std::endl;
                            }
                        }
                        if (!linear_ring) return shared_ptr<RingInfo>();
                    }
                    bool ccw = geos::algorithm::CGAlgorithms::isCCW(linear_ring->getCoordinatesRO());
                    return make_shared<RingInfo>(Osmium::Geometry::geos_geometry_factory()->createPolygon(linear_ring, NULL), ccw ? COUNTERCLOCKWISE : CLOCKWISE);
                } catch (const geos::util::GEOSException& exc) {
                    std::cerr << "Exception: " << exc.what() << std::endl;
                    return shared_ptr<RingInfo>();
                }
            }

            /**
             * Try extending a proto-ring recursively until it is complete.
             */
            shared_ptr<RingInfo> complete_ring(std::vector< shared_ptr<WayInfo> >& ways, osm_object_id_t first, osm_object_id_t last, int ringcount, int sequence) const {

                // is the ring closed already?
                if (first == last) {
                    return ring_is_complete(ways, ringcount, sequence);
                }

                // try extending our current line at the rear end
                for (unsigned int i=0; i<ways.size(); ++i) {
                    // ignore used ways
                    if (ways[i]->used >= 0) continue;

                    // remember old used state in case we have to backtrack
                    int old_used = ways[i]->used;

                    if (ways[i]->firstnode() == last) {
                        // add way to end
                        ways[i]->used = ringcount;
                        ways[i]->sequence = sequence;
                        ways[i]->invert = false;
                        shared_ptr<RingInfo> result = complete_ring(ways, first, ways[i]->lastnode(), ringcount, sequence+1);
                        if (result) {
                            result->ways.push_back(ways[i]);
                            return result;
                        }
                        ways[i]->used = old_used;
                    } else if (ways[i]->lastnode() == last) {
                        // add way to end, but turn it around
                        ways[i]->used = ringcount;
                        ways[i]->sequence = sequence;
                        ways[i]->invert = true;
                        shared_ptr<RingInfo> result = complete_ring(ways, first, ways[i]->firstnode(), ringcount, sequence+1);
                        if (result) {
                            result->ways.push_back(ways[i]);
                            return result;
                        }
                        ways[i]->used = old_used;
                    }
                }

                // we have exhausted all combinations
                return shared_ptr<RingInfo>();
            }

            /**
             * Start with the first available way and build a ring containing it.
             *
             * @returns true if a ring could be built, false otherwise
             */
            bool make_one_ring(std::vector< shared_ptr<WayInfo> >& ways) {
                for (unsigned int i=0; i<ways.size(); ++i) {
                    if (ways[i]->used != -1) continue;
                    ways[i]->used = m_ringlist.size();
                    ways[i]->sequence = 0;
                    ways[i]->invert = false;
                    const shared_ptr<RingInfo>& rl = complete_ring(ways, ways[i]->firstnode(), ways[i]->lastnode(), m_ringlist.size(), 1);
                    if (rl) {
                        rl->ways.push_back(ways[i]);
                        m_ringlist.push_back(rl);
                        return true;
                    }
                    ways[i]->used = -2;
                    break;
                }
                return false;
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
            bool find_and_repair_holes_in_rings(std::vector< shared_ptr<WayInfo> >& ways) const {

                typedef std::vector<Osmium::OSM::WayNode> wnv_t;

                wnv_t dangling_nodes;

                {
                    wnv_t end_nodes;

                    // fill end_nodes vector with all end nodes of all unused ways
                    // and reset way_infos in the process
                    BOOST_FOREACH(shared_ptr<WayInfo>& way_info, ways) {
                        if (way_info->used < 0) {
                            way_info->innerouter = UNSET;
                            way_info->used = -1;
                            end_nodes.push_back(way_info->way->nodes().front());
                            end_nodes.push_back(way_info->way->nodes().back());
                        }
                    }

                    // the env_nodes vector now contains all nodes that are not
                    // open ends twice, sort it so that those nodes are next to
                    // each other
                    std::sort(end_nodes.begin(), end_nodes.end());

                    // find nodes that are not doubled up after the sort and add
                    // them to the dangling_nodes vector
                    for (wnv_t::const_iterator it = end_nodes.begin(); it != end_nodes.end(); ++it) {
                        if ((it+1 == end_nodes.end()) || (*it != *(it+1))) {
                            dangling_nodes.push_back(*it);
                        } else {
                            ++it;
                        }
                    }
                }

                assert(dangling_nodes.size() % 2 == 0);

                // if there are dangling nodes but aren't repairing we return
                // false
                if (!m_attempt_repair && !dangling_nodes.empty()) {
                    return false;
                }

                // while there are any dangling nodes, take the last
                // one and compare the distance to each of the other
                // nodes and find the closest one
                while (!dangling_nodes.empty()) {
                    Osmium::OSM::WayNode wn = dangling_nodes.back();
                    dangling_nodes.pop_back();

                    wnv_t::iterator closest = dangling_nodes.begin();

                    // XXX we probably don't need a haversine here, a simpler distance formula should be ok
                    double min_distance = Osmium::Geometry::Haversine::distance(wn.position(), closest->position());

                    for (wnv_t::iterator it = closest+1; it != dangling_nodes.end(); ++it) {
                        double distance = Osmium::Geometry::Haversine::distance(wn.position(), it->position());
                        if (distance < min_distance) {
                            min_distance = distance;
                            closest = it;
                        }
                    }

                    // create pseudo-way closing the gap
                    shared_ptr<Osmium::OSM::Way> way = make_shared<Osmium::OSM::Way>();
                    way->nodes().push_back(*closest);
                    way->nodes().push_back(wn);
                    ways.push_back(make_shared<WayInfo>(way));
                    std::cerr << "fill gap between nodes " << closest->ref() << " and " << wn.ref() << std::endl;

                    dangling_nodes.erase(closest);
                }

                return true;
            }

            /**
             * Assemble all ways which are members of this relation into a
             * vector of WayInfo elements. this holds room for the way pointer
             * and some extra flags.
             */
            void assemble_ways(std::vector< shared_ptr<WayInfo> >& way_infos) {
                std::map<osm_object_id_t, bool> added_ways;

                BOOST_FOREACH(const shared_ptr<Osmium::OSM::Object const>& object, m_relation_info.members()) {
                    const shared_ptr<Osmium::OSM::Way const> way = static_pointer_cast<Osmium::OSM::Way const>(object);

                    // ignore members that are not ways and ways without nodes
                    if (way && !way->nodes().empty() && (!m_attempt_repair || !added_ways[way->id()])) {
                        if (way->timestamp() > m_new_area->timestamp()) {
                            m_new_area->timestamp(way->timestamp());
                        }
                        added_ways[way->id()] = true;
                        way_infos.push_back(make_shared<WayInfo>(way));
                        // TODO maybe add INNER/OUTER instead of UNSET to enable later warnings on role mismatch
                    }
                }
            }

            /**
             * Try and create as many closed rings as possible from the assortment
             * of ways. make_one_ring will automatically flag those that have been
             * used so they are not used again.
             */
            void make_rings(std::vector< shared_ptr<WayInfo> >& ways) {
                while (make_one_ring(ways)) {
                };

                if (m_ringlist.empty()) {
                    // FIXME throw NoRings("no rings");
                }

                if (!find_and_repair_holes_in_rings(ways)) {
                    throw DanglingEnds("un-connectable dangling ends");
                }

                // re-run ring building, taking into account the newly created "repair" bits.
                // (in case there were no dangling bits, make_one_ring terminates quickly.)
                while (make_one_ring(ways)) {
                };

                if (m_ringlist.empty()) {
                    throw NoRings("no rings");
                }
            }

            /**
             * Find out which ring contains which other ring, so we know
             * which are inner rings and which outer. Don't trust the "role"
             * specifications.
             */
            void determine_inner_outer_rings() {
                typedef boost::dynamic_bitset<> bs_t;

                std::vector<bs_t> contains(m_ringlist.size(), bs_t(m_ringlist.size()));

                bs_t contained_by_even_number(m_ringlist.size());
                contained_by_even_number.set();

                // build contains relationships.
                // we use contained_by_even_number as a helper for us to determine
                // whether something is an inner (false) or outer (true) ring.

                for (unsigned int i=0; i < m_ringlist.size(); ++i) {
                    const scoped_ptr<geos::geom::prep::PreparedPolygon> pp(new geos::geom::prep::PreparedPolygon(m_ringlist[i]->polygon));
                    for (unsigned int j=0; j < m_ringlist.size(); ++j) {
                        if (i==j) continue;
                        if (contains[j][i]) continue;
                        contains[i][j] = pp->contains(m_ringlist[j]->polygon);
                        contained_by_even_number[j] ^= contains[i][j];
                    }
                }

                // we now have an array that has a true value whenever something is
                // contained by something else; if a contains b and b contains c, then
                // our array says that a contains b, b contains c, and a contains c.
                // thin out the array so that only direct relationships remain (and
                // the "a contains c" is dropped).

                for (unsigned int i=0; i < m_ringlist.size(); ++i) {
                    for (unsigned j=0; j < m_ringlist.size(); ++j) {
                        if (contains[i][j]) {
                            // see if there is an intermediary relationship
                            for (unsigned int k=0; k < m_ringlist.size(); ++k) {
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

                for (unsigned int i=0; i < m_ringlist.size(); ++i) {
                    for (unsigned int j=0; j < m_ringlist.size(); ++j) {
                        if (contains[i][j] && !contained_by_even_number[j]) {
                            m_ringlist[j]->contained_by = m_ringlist[i];
                            m_ringlist[i]->inner_rings.push_back(m_ringlist[j]);
                        }
                    }
                }
            }

            void warning(const std::string& /*text*/) const {
//                std::cerr << text << "\n";
            }

            void handle_one_way_inner_ring(RingInfo& ring_info) {
                if (ring_info.ways.size() != 1 || untagged(ring_info.ways[0]->way.get())) {
                    return;
                }

                if (same_tags(ring_info.ways[0]->way.get(), m_new_area.get())) {
                    warning("duplicate_tags_on_inner");
                    return;
                }

                if (ring_info.contained_by.lock()->ways.size() == 1 && same_tags(ring_info.ways[0]->way.get(), ring_info.contained_by.lock()->ways[0]->way.get())) {
                    warning("duplicate_tags_on_inner");
                    return;
                }

                std::vector<geos::geom::Geometry*>* geometries = new std::vector<geos::geom::Geometry*>;

                geometries->push_back(Osmium::Geometry::geos_geometry_factory()->createPolygon(ring_info.ring_in_direction(CLOCKWISE), NULL));

                shared_ptr<Osmium::OSM::Area> internal_area = make_shared<Osmium::OSM::Area>(*(ring_info.ways[0]->way));
                internal_area->geos_geometry(Osmium::Geometry::geos_geometry_factory()->createMultiPolygon(geometries));
                m_areas.push_back(internal_area);
            }

            /**
             * now look at all enclosed (inner) rings that consist of only one way.
             * if such an inner ring has way tags, do the following:
             * - emit an extra polygon for the inner ring if the tags are different
             *   from the relation's
             * - emit a warning, and ignore the inner ring, if the tags are the same
             *   as for the relation
             **/
            int handle_one_way_inner_rings() {
                int outer_ring_count = 0;

                for (unsigned int i=0; i < m_ringlist.size(); ++i) {
                    if (m_ringlist[i]->is_inner()) {
                        handle_one_way_inner_ring(*m_ringlist[i]);
                    } else {
                        ++outer_ring_count;
                    }
                }

                return outer_ring_count;
            }

            void check_touching_inner_rings(const std::vector< shared_ptr<RingInfo> >& inner_rings) const {
                if (inner_rings.empty()) {
                    return;
                }

                for (unsigned int j=0; j < inner_rings.size()-1; ++j) {
                    if (!inner_rings[j]->polygon) continue;
                    const geos::geom::Geometry* ring1_geom = inner_rings[j]->polygon->getExteriorRing();

                    // check if some of the rings touch another ring.
                    for (unsigned int k=j + 1; k < inner_rings.size(); ++k) {
                        if (!inner_rings[k]->polygon) continue;
                        const geos::geom::Geometry* ring2_geom = inner_rings[k]->polygon->getExteriorRing();
                        geos::geom::Geometry* inter = NULL;
                        try {
                            if (!ring1_geom->intersects(ring2_geom)) continue;
                            inter = ring1_geom->intersection(ring2_geom);
                        } catch (const geos::util::GEOSException& exc) {
                            std::cerr << "Exception while checking intersection of rings\n";
                            // nop;
                        }

                        if (inter) {
                            geos::geom::GeometryTypeId type = inter->getGeometryTypeId();
                            delete inter;

                            if (type == geos::geom::GEOS_LINESTRING) {
                                // touching inner rings
                                // this is allowed, but we must fix them up into a valid
                                // geometry
                                geos::geom::Geometry* diff = ring1_geom->symDifference(ring2_geom);
                                const scoped_ptr<geos::operation::polygonize::Polygonizer> polygonizer(new geos::operation::polygonize::Polygonizer());
                                polygonizer->add(diff);
                                std::vector<geos::geom::Polygon*>* polys = polygonizer->getPolygons();
                                if (polys) {
                                    if (polys->size() == 1) {
                                        delete inner_rings[j]->polygon;
                                        inner_rings[j]->polygon = (*polys)[0];
                                        bool ccw = geos::algorithm::CGAlgorithms::isCCW((*polys)[0]->getExteriorRing()->getCoordinatesRO());
                                        inner_rings[j]->direction = ccw ? COUNTERCLOCKWISE : CLOCKWISE;

                                        delete inner_rings[k]->polygon;
                                        inner_rings[k]->polygon = NULL;

                                        delete polys;
                                        check_touching_inner_rings(inner_rings);
                                    } else {
                                        BOOST_FOREACH(geos::geom::Polygon* p, *polys) {
                                            delete p;
                                        }
                                        delete polys;
                                    }
                                    return;
                                }
                            } else {
                                // other kind of intersect between inner rings; this is
                                // not allwoed and will lead to an exception later when
                                // building the MP
                            }
                        }
                    }
                }
            }

            /**
            * Tries to build a multipolygon.
            */
            void build_multipolygon() {
                int outer_ring_count = handle_one_way_inner_rings();

                // for all non-enclosed rings, assemble holes and build polygon.

                std::vector<geos::geom::Geometry*>* polygons = new std::vector<geos::geom::Geometry*>();

                for (unsigned int i=0; i < m_ringlist.size(); ++i) {
                    // look only at outer, i.e. non-contained rings. each ends up as one polygon.
                    if (!m_ringlist[i]) continue; // can happen if ring has been deleted
                    if (m_ringlist[i]->is_inner()) continue;

                    check_touching_inner_rings(m_ringlist[i]->inner_rings);

                    std::vector<geos::geom::Geometry*>* holes = new std::vector<geos::geom::Geometry*>(); // ownership is later transferred to polygon

                    for (unsigned int j=0; j < m_ringlist[i]->inner_rings.size(); ++j) {
                        if (!m_ringlist[i]->inner_rings[j]->polygon) continue;
                        holes->push_back(m_ringlist[i]->inner_rings[j]->ring_in_direction(COUNTERCLOCKWISE));
                    }

                    geos::geom::LinearRing* ring = m_ringlist[i]->ring_in_direction(CLOCKWISE);

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
                        if (p) {
                            delete p;
                        } else {
                            BOOST_FOREACH(geos::geom::Geometry* g, *holes) {
                                delete g;
                            }
                            delete holes;
                            delete ring;
                        }
                        BOOST_FOREACH(geos::geom::Geometry* p, *polygons) {
                            delete p;
                        }
                        delete polygons;
                        throw InvalidRing("invalid ring");
                    } else {
                        polygons->push_back(p);
                        for (unsigned int k=0; k < m_ringlist[i]->ways.size(); ++k) {
                            shared_ptr<WayInfo>& wi = m_ringlist[i]->ways[k];
                            // may have "hole filler" ways in there, not backed by
                            // proper way and thus no tags:
                            if (wi->way == NULL) continue;
                            if (untagged(wi->way.get())) {
                                // way not tagged - ok
                            } else if (same_tags(m_new_area.get(), wi->way.get())) {
                                // way tagged the same as relation/previous ways, ok
                            } else if (untagged(m_new_area.get())) {
                                // relation untagged; use tags from way; ok
                                merge_tags(wi->way.get());
                            } else {
                                // this is grey-area terrain in OSM - we have tags on
                                // the relation and a different set of tags on the outer
                                // way(s). Use tags from outer ring only if there is
                                // only one outer ring and it has only one way.
                                if (outer_ring_count == 1 && m_ringlist[i]->ways.size() == 1) {
                                    merge_tags(wi->way.get());
                                }
                            }

                            wi->innerouter = OUTER;
                        }
                    }
                }

                if (polygons->empty()) {
                    delete polygons;
                    throw NoRings("no rings");
                }

                geos::geom::MultiPolygon* mp = NULL;
                try {
                    mp = Osmium::Geometry::geos_geometry_factory()->createMultiPolygon(polygons);
                } catch (const geos::util::GEOSException& exc) {
                    BOOST_FOREACH(geos::geom::Geometry* p, *polygons) {
                        delete p;
                    }
                    delete polygons;
                    throw InvalidMultiPolygon("multipolygon invalid");
                };

                if (mp->isValid()) {
                    m_new_area->geos_geometry(mp);
                    return;
                }

                delete mp;
                throw InvalidMultiPolygon("multipolygon invalid");
            }

        }; // class Builder

    } // namespace MultiPolygon

} // namespace Osmium

#endif // OSMIUM_MULTIPOLYGON_BUILDER_HPP
