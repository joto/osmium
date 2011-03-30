
#include "osmium.hpp"
#include "Input.hpp"
#include "XMLParser.hpp"
#include "PBFParser.hpp"

#ifdef WITH_GEOS
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

extern bool debug;

namespace Osmium {
    geos::geom::GeometryFactory *geos_factory() {
        static geos::geom::GeometryFactory *global_geometry_factory;

        if (! global_geometry_factory) {
            geos::geom::PrecisionModel *pm = new geos::geom::PrecisionModel();
            global_geometry_factory = new geos::geom::GeometryFactory(pm, -1);
        }

        return global_geometry_factory;
    }
} // namespace Osmium

#endif

// static buffers
char Osmium::OSM::Node::lon_str[];
char Osmium::OSM::Node::lat_str[];

#ifdef WITH_MULTIPOLYGON_PROFILING
std::vector<std::pair<std::string, timer *> > Osmium::OSM::MultipolygonFromRelation::timers;

timer Osmium::OSM::MultipolygonFromRelation::write_complex_poly_timer;
timer Osmium::OSM::MultipolygonFromRelation::assemble_ways_timer;
timer Osmium::OSM::MultipolygonFromRelation::assemble_nodes_timer;
timer Osmium::OSM::MultipolygonFromRelation::make_one_ring_timer;
timer Osmium::OSM::MultipolygonFromRelation::mor_polygonizer_timer;
timer Osmium::OSM::MultipolygonFromRelation::mor_union_timer;
timer Osmium::OSM::MultipolygonFromRelation::contains_timer;
timer Osmium::OSM::MultipolygonFromRelation::extra_polygons_timer;
timer Osmium::OSM::MultipolygonFromRelation::polygon_build_timer;
timer Osmium::OSM::MultipolygonFromRelation::inner_ring_touch_timer;
timer Osmium::OSM::MultipolygonFromRelation::polygon_intersection_timer;
timer Osmium::OSM::MultipolygonFromRelation::multipolygon_build_timer;
timer Osmium::OSM::MultipolygonFromRelation::multipolygon_write_timer;
timer Osmium::OSM::MultipolygonFromRelation::error_write_timer;
#endif

#include <fcntl.h>

