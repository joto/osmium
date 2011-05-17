/*

  Convert OSM files from one format into another.

*/

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <cstdlib>
#include <getopt.h>

#define OSMIUM_MAIN
#include <osmium.hpp>

class ConvertHandler : public Osmium::Handler::Base {

    Osmium::Output::OSM::Base *output;

    std::string filename;

public:

    ConvertHandler() : filename("-") {
    }

    ConvertHandler(std::string &fn) : filename(fn) {
    }

    void callback_init() {
        output = Osmium::Output::OSM::create(filename);
        output->write_init();
    }

    void callback_node(Osmium::OSM::Node *node) {
        output->write(node);
    }

    void callback_way(Osmium::OSM::Way *way) {
        output->write(way);
    }

    void callback_relation(Osmium::OSM::Relation *relation) {
        output->write(relation);
    }

    void callback_final() {
        output->write_final();
        delete output;
    }

};

/* ================================================== */

void print_help() {
    std::cout << "osmium_convert [OPTIONS] INFILE OUTFILE" << std::endl \
              << "Options:" << std::endl \
              << "  --help, -h     - This help message" << std::endl \
              << "  --debug, -d    - Enable debugging output" << std::endl;
}

int main(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"debug",                no_argument, 0, 'd'},
        {"help",                 no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    bool debug = false;

    while (1) {
        int c = getopt_long(argc, argv, "dh", long_options, 0);
        if (c == -1)
            break;

        switch (c) {
            case 'd':
                debug = true;
                break;
            case 'h':
                print_help();
                exit(0);
            default:
                exit(1);
        }
    }

    if (optind > argc-2) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] INFILE OUTFILE" << std::endl;
        exit(1);
    }

    Osmium::Framework osmium(debug);

    std::string outfile(argv[optind+1]);

    ConvertHandler *handler_convert = new ConvertHandler(outfile);

    osmium.parse_osmfile<ConvertHandler>(argv[optind], handler_convert);

    delete handler_convert;
}

