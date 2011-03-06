#ifndef OSMIUM_HANDLER_NODE_DENSITY_HPP
#define OSMIUM_HANDLER_NODE_DENSITY_HPP

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
          
            NodeDensity(int _size, int _min, int _max) : Base(0), xsize(_size*2), ysize(_size), min(_min), max(_max) {
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

                int colors[256];
                for (int i=0; i <= 255; i++) {
                    colors[i] = gdImageColorAllocate(im, i, i, i);
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
            }

        }; // class NodeDensity

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_NODE_DENSITY_HPP
