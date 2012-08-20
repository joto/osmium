#ifndef OSMIUM_OSMIUM_HPP
#define OSMIUM_OSMIUM_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#ifdef OSMIUM_WITH_PBF_INPUT
# include <osmium/input/pbf.hpp>
#endif

#ifdef OSMIUM_WITH_XML_INPUT
# include <osmium/input/xml.hpp>
#endif

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

/**
 * @brief All %Osmium code is in this namespace.
 */
namespace Osmium {

#if defined(OSMIUM_WITH_PBF_INPUT) || defined(OSMIUM_WITH_XML_INPUT)
    namespace Input {

        template <class T>
        inline void read(const Osmium::OSMFile& file, T& handler) {
            Osmium::Input::Base<T>* input = NULL;

            if (file.encoding()->is_pbf()) {
#ifdef OSMIUM_WITH_PBF_INPUT
                input = static_cast<Osmium::Input::Base<T>*>(new Osmium::Input::PBF<T>(file, handler));
#else
                throw Osmium::OSMFile::FileEncodingNotSupported();
#endif // OSMIUM_WITH_PBF_INPUT
            } else {
#ifdef OSMIUM_WITH_XML_INPUT
                input = static_cast<Osmium::Input::Base<T>*>(new Osmium::Input::XML<T>(file, handler));
#else
                throw Osmium::OSMFile::FileEncodingNotSupported();
#endif // OSMIUM_WITH_XML_INPUT
            }

            input->parse();
            delete input;
        }

    } // namespace Input
#endif

} // namespace Osmium

#endif // OSMIUM_OSMIUM_HPP
