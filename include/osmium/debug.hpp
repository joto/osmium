#ifndef OSMIUM_DEBUG_HPP
#define OSMIUM_DEBUG_HPP

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
     * This class can be used as a base class for classes that need
     * debugging support.
     *
     * To set the debug level: set_debug_level(LEVEL);
     *
     * Check whether debugging is wanted: if (debug && has_debug_level(LEVEL)) ...
     *
     * The const "debug" is set to "true" or "false" depending on whether
     * Osmium was compiled with or without debugging support, so the debugging
     * code can be optimized out.
     */
    class WithDebug {

        int m_debug_level;

    protected:

#ifdef OSMIUM_WITH_DEBUG
        static const bool debug = true;
#else
        static const bool debug = false;
#endif // OSMIUM_WITH_DEBUG

    public:

        WithDebug() :
            m_debug_level(0) {
        }

        // Destructor is not virtual as this class is not intended to be used polymorphically
        ~WithDebug() {
        }

        bool has_debug_level(int debug_level) const {
            return m_debug_level >= debug_level;
        }

        void set_debug_level(int debug_level) {
            m_debug_level = debug_level;
        }

    }; // class WithDebug

} // namespace Osmium

#endif // OSMIUM_DEBUG_HPP
