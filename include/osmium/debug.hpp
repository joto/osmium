#ifndef OSMIUM_DEBUG_LEVEL_HPP
#define OSMIUM_DEBUG_LEVEL_HPP

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

namespace Osmium {

    /**
     * This class can be used as a base class for classes that need a
     * debug_level setting.
     */
    class WithDebugLevel {

        int m_debug_level;

    public:

        WithDebugLevel() : m_debug_level(0) {
        }

        int debug_level() const {
            return m_debug_level;
        }

        void debug_level(int debug_level) {
            m_debug_level = debug_level;
        }

    };

} // namespace Osmium

/**
 * If Osmium is compiled with OSMIUM_WITH_DEBUG, the OSMIUM_DEBUG macro is
 * defined to check the debug_level() set on the current object and if it
 * is high enough to send the debug message to stdout.
 */
#ifdef OSMIUM_WITH_DEBUG
# define OSMIUM_DEBUG(lvl, msg) \
    if (debug_level() >= lvl) { \
        std::cout << msg; \
    } else // eats the semikolon after the call
#else
# define OSMIUM_DEBUG(level, msg)
#endif // OSMIUM_WITH_DEBUG

#endif // OSMIUM_DEBUG_LEVEL_HPP
