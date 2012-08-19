#ifndef OSMIUM_SMART_PTR_HPP
#define OSMIUM_SMART_PTR_HPP

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

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>

using boost::shared_ptr;
using boost::weak_ptr;
using boost::scoped_ptr;
using boost::static_pointer_cast;
using boost::const_pointer_cast;
using boost::dynamic_pointer_cast;
using boost::make_shared;

#endif // OSMIUM_SMART_PTR_HPP
