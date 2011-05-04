/**********************************************************************
 * $Id: CGAlgorithms.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/CGAlgorithms.java rev. 1.34 (JTS-1.7.1)
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_CGALGORITHM_H
#define GEOS_ALGORITHM_CGALGORITHM_H

#include <vector>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
		class CoordinateSequence;
	}
}


namespace geos {
namespace algorithm { // geos::algorithm

/**
 * \brief
 * Specifies and implements various fundamental Computational Geometric
 * algorithms.
 * The algorithms supplied in this class are robust for double-precision
 * floating point.
 *
 */
class CGAlgorithms {

public:

	enum {
		CLOCKWISE=-1,
		COLLINEAR,
		COUNTERCLOCKWISE
	};

	enum {
		RIGHT=-1,
		LEFT,
		STRAIGHT
	};

	CGAlgorithms(){};

	/** \brief
	 * Test whether a point lies inside a ring.
	 *
	 * The ring may be oriented in either direction.
	 * If the point lies on the ring boundary the result
	 * of this method is unspecified.
	 * 
	 * This algorithm does not attempt to first check the
	 * point against the envelope of the ring.
	 *
	 * @param p point to check for ring inclusion
	 * @param ring assumed to have first point identical to last point
	 * @return <code>true</code> if p is inside ring
	 */
	static bool isPointInRing(const geom::Coordinate& p,
			const geom::CoordinateSequence* ring);

	/// Same as above, but taking a vector of const Coordinates (faster)
	static bool isPointInRing(const geom::Coordinate& p,
			const std::vector<const geom::Coordinate*>& ring);

	/** \brief
	 * Test whether a point lies on a linestring.
	 *
	 * @return true true if
	 * the point is a vertex of the line or lies in the interior of a line
	 * segment in the linestring
	 */
	static bool isOnLine(const geom::Coordinate& p,
		const geom::CoordinateSequence* pt);

	/** \brief
	 * Computes whether a ring defined by an array of Coordinate is
	 * oriented counter-clockwise.
	 * 
	 *  - The list of points is assumed to have the first and last
	 *    points equal.
	 *  - This will handle coordinate lists which contain repeated points.
	 *
	 * This algorithm is <b>only</b> guaranteed to work with valid rings.
	 * If the ring is invalid (e.g. self-crosses or touches),
	 * the computed result <b>may</b> not be correct.
	 *
	 * @param ring an array of coordinates forming a ring
	 * @return <code>true</code> if the ring is oriented counter-clockwise.
	 */
	static bool isCCW(const geom::CoordinateSequence* ring);

	/** \brief
	 * Computes the orientation of a point q to the directed line
	 * segment p1-p2.
	 *
	 * The orientation of a point relative to a directed line
	 * segment indicates which way you turn to get to q after
	 * travelling from p1 to p2.
	 *
	 * @return 1 if q is counter-clockwise from p1-p2
	 * @return -1 if q is clockwise from p1-p2
	 * @return 0 if q is collinear with p1-p2
	 */
	static int computeOrientation(const geom::Coordinate& p1,
			const geom::Coordinate& p2,
			const geom::Coordinate& q);

	/** \brief
	 * Computes the distance from a point p to a line segment AB
	 *
	 * Note: NON-ROBUST!
	 *
	 * @param p the point to compute the distance for
	 * @param A one point of the line
	 * @param B another point of the line (must be different to A)
	 * @return the distance from p to line segment AB
	 */
	static double distancePointLine(const geom::Coordinate& p,
			const geom::Coordinate& A,
			const geom::Coordinate& B);

	/** \brief
	 * Computes the perpendicular distance from a point p
	 * to the (infinite) line containing the points AB
	 *
	 * @param p the point to compute the distance for
	 * @param A one point of the line
	 * @param B another point of the line (must be different to A)
	 * @return the distance from p to line AB
	 */
	static double distancePointLinePerpendicular(const geom::Coordinate& p,
			const geom::Coordinate& A,
			const geom::Coordinate& B);

	/** \brief
	 * Computes the distance from a line segment AB to a line segment CD
	 *
	 * Note: NON-ROBUST!
	 *
	 * @param A a point of one line
	 * @param B the second point of  (must be different to A)
	 * @param C one point of the line
	 * @param D another point of the line (must be different to A)
	 */
	static double distanceLineLine(const geom::Coordinate& A,
			const geom::Coordinate& B,
			const geom::Coordinate& C,
			const geom::Coordinate& D);

	/** \brief
	 * Returns the signed area for a ring.  The area is positive if
	 * the ring is oriented CW.
	 */
	static double signedArea(const geom::CoordinateSequence* ring);

	/** \brief
	 * Computes the length of a linestring specified by a sequence
	 * of points.
	 *
	 * @param pts the points specifying the linestring
	 * @return the length of the linestring
	 */
	static double length(const geom::CoordinateSequence* pts);

	/** \brief
	 * Returns the index of the direction of the point <code>q</code>
	 * relative to a vector specified by <code>p1-p2</code>.
	 *
	 * @param p1 the origin point of the vector
	 * @param p2 the final point of the vector
	 * @param q the point to compute the direction to
	 *
	 * @return 1 if q is counter-clockwise (left) from p1-p2
	 * @return -1 if q is clockwise (right) from p1-p2
	 * @return 0 if q is collinear with p1-p2
	 */
	static int orientationIndex(const geom::Coordinate& p1,
			const geom::Coordinate& p2,
			const geom::Coordinate& q);

};

} // namespace geos::algorithm
} // namespace geos

#endif // GEOS_ALGORITHM_CGALGORITHM_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/05/02 14:51:53  strk
 * Added port info and fixed doxygen comments for CGAlgorithms class
 *
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

