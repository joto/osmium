#ifndef OSMIUM_STORAGE_BYID_HPP
#define OSMIUM_STORAGE_BYID_HPP

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

#include <stdint.h>
#include <boost/utility.hpp>

namespace Osmium {

    /**
     * @brief Classes handling storage of data.
     */
    namespace Storage {

        namespace ById {

            /**
            * This abstract class defines an interface to storage classes
            * intended for storing small pieces of data (such as coordinates)
            * indexed by a positive object ID. The storage must be very
            * space efficient and able to scale to billions of objects.
            *
            * Subclasses have different implementations that will store the
            * data in different ways in memory and/or on disk. Some storage
            * classes are better suited when working with the whole planet,
            * some are better for data extracts.
            *
            * Note that these classes are not required to track "empty" fields.
            * When reading data you have to be sure you have put something in
            * there before.
            *
            * This storage class will only work on 64 bit systems if used for
            * storing node coordinates. 32 bit systems just can't address
            * that much memory!
            */
            template <typename TValue>
            class Base : boost::noncopyable {

            public:

                virtual ~Base() {
                }

                /// The "value" type, usually a coordinates class or similar.
                typedef TValue value_type;

                /// Set the field with id to value.
                virtual void set(const uint64_t id, const TValue value) = 0;

                /// Retrieve value by key. Does not check for overflow or empty fields.
                virtual const TValue operator[](const uint64_t id) const = 0;

                /**
                * Get the approximate number of items in the storage. The storage
                * might allocate memory in blocks, so this size might not be
                * accurate. You can not use this to find out how much memory the
                * storage uses. Use used_memory() for that.
                */
                virtual uint64_t size() const = 0;

                /**
                * Get the memory used for this storage in bytes. Note that this
                * is not necessarily entirely accurate but an approximation.
                * For storage classes that store the data in memory, this is
                * the main memory used, for storage classes storing data on disk
                * this is the memory used on disk.
                */
                virtual uint64_t used_memory() const = 0;

                /**
                * Clear memory used for this storage. After this you can not
                * use the storage container any more.
                */
                virtual void clear() = 0;

            }; // class Base

        } // namespace ById

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_STORAGE_BYID_HPP
