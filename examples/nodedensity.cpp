/*
The code in this example file is released into the Public Domain.
*/

#include <limits>
#include <iostream>
#include <gd.h>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>

/* ================================================== */

typedef uint16_t node_count_t;

class NodeDensityHandler : public Osmium::Handler::Base {

    node_count_t* m_node_count;
    int m_xsize;
    int m_ysize;
    double m_factor;
    int m_min;
    int m_max;
    int m_diff;
    int m_max_count;

public:

    NodeDensityHandler(int size = 1024, int min = 0, int max = 99999)
        : Base(),
          m_xsize(size*2),
          m_ysize(size),
          m_min(min),
          m_max(max) {
        m_factor = m_ysize / 180;
        m_node_count = static_cast<node_count_t*>(calloc(m_xsize * m_ysize, sizeof(node_count_t)));
        m_max_count = 0;
        m_diff = m_max - m_min;
    }

    ~NodeDensityHandler() {
        free(m_node_count);
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& node) {
        int x = int((180 + node->position().lon()) * m_factor);
        int y = int(( 90 - node->position().lat()) * m_factor);
        if (x <        0) x =         0;
        if (x >= m_xsize) x = m_xsize-1;
        if (y <        0) y =         0;
        if (y >= m_ysize) y = m_ysize-1;
        int n = y * m_xsize + x;
        if (m_node_count[n] < std::numeric_limits<node_count_t>::max() - 1) {
            m_node_count[n]++;
        }
        if (m_node_count[n] > m_max_count) {
            m_max_count = m_node_count[n];
        }
    }

    void after_nodes() {
        std::cerr << "max_count=" << m_max_count << "\n";
        gdImagePtr im = gdImageCreate(m_xsize, m_ysize);

        for (int i=0; i <= 255; ++i) {
            gdImageColorAllocate(im, i, i, i);
        }

        int n=0;
        for (int y=0; y < m_ysize; ++y) {
            for (int x=0; x < m_xsize; ++x) {
                int val = m_node_count[n++];
                if (val < m_min) val = m_min;
                if (val > m_max) val = m_max;
                gdImageSetPixel(im, x, y, static_cast<uint8_t>((val - m_min) * 255 / m_diff));
            }
        }

        gdImagePng(im, stdout);
        gdImageDestroy(im);
        throw Osmium::Handler::StopReading();
    }

}; // class NodeDensityHandler

/* ================================================== */

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE SIZE MIN MAX\n\n";
        std::cerr << "  OSMFILE - OSM file of any type.\n";
        std::cerr << "  SIZE    - Y-size of resulting image (X-size will be double).\n";
        std::cerr << "  MIN     - Node counts smaller than this will be black.\n";
        std::cerr << "  MAX     - Node counts larger than this will be white.\n\n";
        std::cerr << "Output will be a PNG file on STDOUT. Make sure to redirect.\n";
        exit(1);
    }

    int size = atoi(argv[2]);
    int min  = atoi(argv[3]);
    int max  = atoi(argv[4]);

    Osmium::OSMFile infile(argv[1]);
    NodeDensityHandler handler(size, min, max);
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

