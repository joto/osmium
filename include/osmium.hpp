#ifndef OSMIUM_OSMIUM_HPP
#define OSMIUM_OSMIUM_HPP

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

/**
 * @mainpage
 *
 * %Osmium is a fast and flexible C++ and Javascript toolkit and framework for
 * working with OSM data.
 *
 * This is the API documentation that was automatically created from the
 * source code. For more general information see
 * http://wiki.openstreetmap.org/wiki/Osmium .
 *
 * %Osmium is free software and available under the LGPLv3 or GPLv3. The
 * source code is at https://github.com/joto/osmium .
 */


#ifdef OSMIUM_WITH_JAVASCRIPT
# include <v8.h>
# include <unicode/ustring.h>
# include <osmium/utils/unicode.hpp>
#endif // OSMIUM_WITH_JAVASCRIPT

#include <osmpbf/osmpbf.h>

/**
 * @brief All %Osmium code is in this namespace.
 */
namespace Osmium {

    /**
     * Internal class to manage global state.
     */
    class Framework {

        Framework(bool d) : debug(d) {
        }

        ~Framework() {
            // this is needed even if the protobuf lib was never used so that valgrind doesn't report any errors
            google::protobuf::ShutdownProtobufLibrary();
        }

        bool debug;

        friend Framework& init(bool debug);
        friend void set_debug(bool d);
        friend bool debug();

    }; // class Framework

    /**
     * Initialize the Osmium library. Call this before using any of the Osmium
     * functions.
     *
     * @param debug Enable or disable the debugging output.
     */
    Framework& init(bool debug=false) {
        static Framework f(debug);
        return f;
    }

    /**
     * Enable or disable the debugging output.
     */
    void set_debug(bool d) {
        init().debug = d;
    }

    /**
     * Is debugging output set?
     */
    bool debug() {
        return init().debug;
    }

} // namespace Osmium

// check way geometry before making a shplib object from it
// normally this should be defined, otherwise you will generate invalid linestring geometries
#define OSMIUM_CHECK_WAY_GEOMETRY

#ifdef OSMIUM_WITH_JAVASCRIPT
# include <osmium/javascript/template.hpp>
#endif // OSMIUM_WITH_JAVASCRIPT

#include <osmium/exceptions.hpp>
#include <osmium/osm.hpp>
#include <osmium/geometry/null.hpp>
#include <osmium/geometry/point.hpp>
#include <osmium/geometry/linestring.hpp>
#include <osmium/geometry/polygon.hpp>
#include <osmium/geometry/multipolygon.hpp>
#include <osmium/osmfile.hpp>
#include <osmium/input.hpp>
#include <osmium/output.hpp>
#include <osmium/export.hpp>
#include <osmium/osmfile_impl.hpp>

#ifdef OSMIUM_WITH_JAVASCRIPT
# include <osmium/HandlerJavascript.hpp>
#endif // OSMIUM_WITH_JAVASCRIPT

#endif // OSMIUM_OSMIUM_HPP
