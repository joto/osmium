#ifndef OSMIUM_GEOMETRY_FACTORY_HPP
#define OSMIUM_GEOMETRY_FACTORY_HPP

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

namespace Osmium {
    namespace Geometry {
        /**
         * Return pointer to a static GEOS GeometryFactory object created the
         * first time this function is run. This is used by all functions in
         * Osmium that need to create GEOS geometries.
         */
        geos::geom::GeometryFactory* geos_geometry_factory() {
            static geos::geom::PrecisionModel pm;
            static geos::geom::GeometryFactory factory(&pm, -1);
            return &factory;
        }
    }
}

#endif // OSMIUM_GEOMETRY_FACTORY_HPP
