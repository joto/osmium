#ifndef OSMIUM_JAVASCRIPT_WRAPPER_POSITION_HPP
#define OSMIUM_JAVASCRIPT_WRAPPER_POSITION_HPP

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

#include <v8.h>

#include <osmium/javascript/unicode.hpp>
#include <osmium/javascript/template.hpp>
#include <osmium/osm/position.hpp>

namespace Osmium {

    namespace Javascript {

        namespace Wrapper {

            struct OSMPosition {

                static v8::Handle<v8::Array> to_array(const Osmium::OSM::Position& position) {
                    v8::HandleScope scope;
                    v8::Local<v8::Array> array = v8::Array::New(2);
                    array->Set(0, v8::Number::New(position.lon()));
                    array->Set(1, v8::Number::New(position.lat()));
                    return scope.Close(array);
                }

            };

        } // namespace Wrapper

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_WRAPPER_POSITION_HPP
