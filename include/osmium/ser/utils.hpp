#ifndef OSMIUM_SER_UTILS_HPP
#define OSMIUM_SER_UTILS_HPP

/*

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <cerrno>
#include <unistd.h>
#include <string>

namespace Osmium {

    namespace Ser {

        /**
         * Wrapper for write(2) system call.
         */
        void write(int fd, const void* buf, size_t count) {
            size_t offset = 0;
            do {
                ssize_t length = ::write(fd, static_cast<const char*>(buf) + offset, count);
                if (length == -1) {
                    throw std::runtime_error(std::string("Write error: ") + strerror(errno));
                }
                offset += length;
                count -= length;
            } while (count > 0);
        }

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_UTILS_HPP
