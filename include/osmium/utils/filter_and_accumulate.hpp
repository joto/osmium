#ifndef OSMIUM_UTILS_FILTER_AND_ACCUMULATE_HPP
#define OSMIUM_UTILS_FILTER_AND_ACCUMULATE_HPP

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

#include <numeric>

namespace Osmium {

    /**
     * Similar to the std::accumulate function, but first filters while
     * iterating over the container.
     *
     * @param container Some container class, must support begin() and end() functions.
     * @param filter Filter class, must support operator() that takes a member of the container and returns bool.
     * @param init Initial value for accumulation.
     * @param binary_op Operation called when accumulating.
     */
    template <class TContainer, class TFilter, class TAccum, class TBinaryOp>
    inline TAccum filter_and_accumulate(TContainer& container, TFilter& filter, const TAccum& init, TBinaryOp binary_op) {
        typename TFilter::iterator fi_begin(filter, container.begin(), container.end());
        typename TFilter::iterator fi_end(filter, container.end(), container.end());

        return std::accumulate(fi_begin, fi_end, init, binary_op);
    }

} // namespace Osmium

#endif // OSMIUM_UTILS_FILTER_AND_ACCUMULATE_HPP
