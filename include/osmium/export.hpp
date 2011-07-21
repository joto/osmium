#ifndef OSMIUM_EXPORT_HPP
#define OSMIUM_EXPORT_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

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

namespace Osmium {

    /**
     * @brief Classes implementing export into non-OSM formats such as to shapefiles.
     */
    namespace Export {
    } // namespace Export

} // namespace Osmium

#ifdef OSMIUM_WITH_JAVASCRIPT
# include <osmium/export/csv.hpp>
# include <osmium/export/shapefile.hpp>
#endif

#endif // OSMIUM_EXPORT_HPP
