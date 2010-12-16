
#ifdef WITH_GEOS
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

#include "osmium.hpp"

namespace Osmium {

    namespace OSM {

        geos::geom::GeometryFactory *Object::global_geometry_factory = NULL;

    }

}
#endif

