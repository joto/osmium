#ifndef OSMIUM_OSMIUM_HPP
#define OSMIUM_OSMIUM_HPP

// check way geometry before making a shplib object from it
// normally this should be defined, otherwise you will generate invalid linestring geometries
#define CHECK_WAY_GEOMETRY

#ifdef WITH_GEOS
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

namespace Osmium {
    geos::geom::GeometryFactory *geos_factory();
} // namespace Osmium

#endif

#include <osmium/osm.hpp>
#include <osmium/input.hpp>
#include <osmium/framework.hpp>

#ifdef WITH_JAVASCRIPT
# include <Javascript.hpp>
#endif

#ifdef WITH_GEOS
#ifndef IN_JAVASCRIPT_TEMPLATE
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
#endif // WITH_GEOS

#endif // OSMIUM_OSMIUM_HPP
