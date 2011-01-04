#ifndef OSMIUM_OSM_WAY_HPP
#define OSMIUM_OSM_WAY_HPP

#ifdef WITH_SHPLIB
#include <shapefil.h>
#endif

/** @file
*   @brief Contains the Osmium::OSM::Way class.
*/

#ifdef WITH_GEOS
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/util/GEOSException.h>
#endif

#include <stdexcept>

namespace Osmium {

    namespace OSM {

        class Way : public Object {

            osm_sequence_id_t num_nodes;

          public:

            static const int max_nodes_in_way = 2000; ///< There can only be 2000 nodes in a way as per OSM API 0.6 definition.

            osm_object_id_t *nodes;
            double          *lon;
            double          *lat;

            // TODO XXX temporary for multipoly integration
            bool tried;
            bool size_frozen;

            // construct a Way object with full node capacity.
            Way() : Object() {
                nodes = (osm_object_id_t *) malloc(sizeof(osm_object_id_t) * max_nodes_in_way);
                lon = (double *) malloc(sizeof(double) * max_nodes_in_way);
                lat = (double *) malloc(sizeof(double) * max_nodes_in_way);
                size_frozen = false;
                reset();
            }

            // copy a Way object. allocate only the required capacity.
            Way(const Way& w) : Object(w) {
                size_t s;
                num_nodes = w.num_nodes;
                nodes = (osm_object_id_t *) malloc(s = sizeof(osm_object_id_t) * num_nodes);
                memcpy(nodes, w.nodes, s);
                lon = (double *) malloc(s = sizeof(double) * num_nodes);
                memcpy(lon, w.lon, s);
                lat = (double *) malloc(s = sizeof(double) * num_nodes);
                memcpy(lat, w.lat, s);
                size_frozen = true;
            }

            ~Way() {
                free(nodes);
                free(lon);
                free(lat);
            }

            osm_object_type_t get_type() const {
                return WAY;
            }

            void reset() {
                Object::reset();
                num_nodes = 0;
                tried = false;
            }

            /**
            * Add a node with the given id to the way.
            *
            * Will throw a range error if the way already has max_nodes_in_way nodes.
            */
            void add_node(osm_object_id_t ref) {
                if (size_frozen) {
                    throw std::range_error("cannot add nodes to frozen way");
                }
                if (num_nodes < max_nodes_in_way) {
                    nodes[num_nodes++] = ref;
                }
                else {
                    throw std::range_error("no more than 2000 nodes in a way");
                }
            }

            /**
            * Returns the number of nodes in this way.
            */
            osm_sequence_id_t node_count() const {
                return num_nodes;
            }

            /** 
             * Returns the id of the first node.
             */
            osm_object_id_t get_first_node_id() {
                return nodes[0];
            }

            /** 
             * Returns the id of the last node.
             */
            osm_object_id_t get_last_node_id() {
                return nodes[num_nodes - 1];
            }

#ifdef WITH_GEOS
            /** 
             * Returns the GEOS geometry of the first node.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Point *get_first_node_geometry() {
                geos::geom::Coordinate c;
                c.x = lon[0];
                c.y = lat[0];
                return Osmium::geos_factory()->createPoint(c);
            }

            /** 
             * Returns the GEOS geometry of the last node.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Point *get_last_node_geometry() {
                geos::geom::Coordinate c;
                c.x = lon[num_nodes - 1];
                c.y = lat[num_nodes - 1];
                return Osmium::geos_factory()->createPoint(c);
            }
#endif

            /**
            * Check whether this way is closed. A way is closed if the first and last node have the same id.
            */
            bool is_closed() const {
                return nodes[0] == nodes[num_nodes-1];
            }

            /**
            * Set coordinates for the nth node in this way.
            */
            void set_node_coordinates(osm_sequence_id_t n, double nlon, double nlat) {
                assert(0 <= n && n < num_nodes);
                lon[n] = nlon;
                lat[n] = nlat;
            }

#ifdef WITH_GEOS
            /** 
             * Returns the GEOS geometry of the way.
             * Caller takes ownership of the pointer.
             */
            geos::geom::Geometry *create_geos_geometry() {
                try {
                    std::vector<geos::geom::Coordinate> *c = new std::vector<geos::geom::Coordinate>;
                    for (int i=0; i<num_nodes; i++) {
                        c->push_back(geos::geom::Coordinate(lon[i], lat[i], DoubleNotANumber));
                    }
                    geos::geom::CoordinateSequence *cs = Osmium::geos_factory()->getCoordinateSequenceFactory()->create(c);
                    return (geos::geom::Geometry *) Osmium::geos_factory()->createLineString(cs);
                } catch (const geos::util::GEOSException& exc) {
                    std::cerr << "error building way geometry, leave it as NULL" << std::endl;
                    return NULL;
                }
            }
#endif

#ifdef WITH_SHPLIB
            /**
            * Create a SHPObject for this way and return it. You have to call
            * SHPDestroyObject() with this object when you are done.
            */
            SHPObject *create_shpobject(int shp_type) {
                if (shp_type != SHPT_ARC && shp_type != SHPT_POLYGON) {
                    throw std::runtime_error("a way can only be added to a shapefile of type line or polygon");
                }
                return SHPCreateSimpleObject(shp_type, num_nodes, lon, lat, NULL);
            }
#endif

        }; // class Way

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_WAY_HPP
