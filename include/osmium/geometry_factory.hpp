#ifndef OSMIUM_GEOMETRY_FACTORY_HPP
#define OSMIUM_GEOMETRY_FACTORY_HPP

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Coordinate.h>

#include <osmium/osm/position.hpp>

namespace Osmium {
    namespace Geometry {

        /**
         * Return pointer to a static GEOS GeometryFactory object created the
         * first time this function is run. This is used by all functions in
         * Osmium that need to create GEOS geometries.
         */
        inline geos::geom::GeometryFactory* geos_geometry_factory() {
            static geos::geom::PrecisionModel pm;
            static geos::geom::GeometryFactory factory(&pm, -1);
            return &factory;
        }

        /**
         * Creates GEOS coordinate from a Position
         */
        inline geos::geom::Coordinate create_geos_coordinate(const Osmium::OSM::Position position) {
            return geos::geom::Coordinate(position.lon(), position.lat());
        }

    }
}

#endif // OSMIUM_GEOMETRY_FACTORY_HPP
