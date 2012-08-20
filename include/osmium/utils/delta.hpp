#ifndef OSMIUM_UTILS_DELTA_HPP
#define OSMIUM_UTILS_DELTA_HPP

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

#include <algorithm>

namespace Osmium {

    /**
     * This class models a variable that keeps track of the value
     * it was last set to and returns the delta between old and
     * new value from the update() call.
     */
    template <typename T>
    class Delta {

    public:

        Delta() :
            m_value(0) {
        }

        void clear() {
            m_value = 0;
        }

        T update(T new_value) {
            std::swap(m_value, new_value);
            return m_value - new_value;
        }

    private:

        T m_value;

    };

}

#endif // OSMIUM_UTILS_DELTA_HPP
