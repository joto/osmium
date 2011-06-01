#ifndef OSMIUM_HANDLER_NODE_DENSITY_HPP
#define OSMIUM_HANDLER_NODE_DENSITY_HPP

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

#include <string>

#include <stdio.h>
#include <gd.h>

typedef uint16_t node_count_t;
#define MAX_NODE_COUNT 0xfffe

namespace Osmium {

    namespace Handler {

        class NodeDensity : public Base {

            node_count_t *node_count;
            int xsize;
            int ysize;
            double factor;
            int min, max;
            int max_count;

        public:

            NodeDensity(int _size = 1024, int _min = 0, int _max = 99999) : Base(), xsize(_size*2), ysize(_size), min(_min), max(_max) {
                factor = ysize / 180;
                node_count = (node_count_t *) calloc(xsize * ysize, sizeof(uint16_t));
                max_count = 0;
            }

            ~NodeDensity() {
                free(node_count);
            }

            void callback_node(Osmium::OSM::Node *node) {
                int x = int( (180 + node->get_lon()) * factor );
                int y = int( ( 90 - node->get_lat()) * factor );
                if (x <      0) x =       0;
                if (x >= xsize) x = xsize-1;
                if (y <      0) y =       0;
                if (y >= ysize) y = ysize-1;
                int n = y * xsize + x;
                if (node_count[n] < MAX_NODE_COUNT) {
                    node_count[n]++;
                }
                if (node_count[n] > max_count) {
                    max_count = node_count[n];
                }
            }

            void callback_after_nodes() {
                std::cerr << "max_count=" << max_count << "\n";
                gdImagePtr im = gdImageCreate(xsize, ysize);

                for (int i=0; i <= 255; i++) {
                    gdImageColorAllocate(im, i, i, i);
                }

                int n=0;
                for (int y=0; y < ysize; y++) {
                    for (int x=0; x < xsize; x++) {
                        int val = node_count[n++];
                        if (val < min) val = min;
                        if (val > max) val = max;
                        gdImageSetPixel(im, x, y, uint8_t((val-min) * 255 / (max-min)));
                    }
                }
                gdImagePng(im, stdout);
                gdImageDestroy(im);
                throw Osmium::Input::StopReading();
            }

        }; // class NodeDensity

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_NODE_DENSITY_HPP
