#ifndef geoshelper_h
#define geoshelper_h

#include <geos/geom/MultiPoint.h>
#include <geos/geom/Point.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
//#include <geos/operation/EndpointInfo.h>

#include <map>

class EndpointInfo;

class GeosHelper
{

    public:

    static geos::geom::Point *selfIntersection(geos::geom::Geometry *g, geos::geom::GeometryFactory *fac);
    static geos::geom::MultiPoint *selfIntersections(geos::geom::Geometry *g, geos::geom::GeometryFactory *fac);

    private:
     
    static void addEndpoint(std::map<const geos::geom::Coordinate*,EndpointInfo*,geos::geom::CoordinateLessThen>&endPoints, const geos::geom::Coordinate *p,bool isClosed);


};

#endif
