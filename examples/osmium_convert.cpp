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
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <cstdlib>
#include <getopt.h>

#define OSMIUM_MAIN
#include <osmium.hpp>

class ConvertHandler : public Osmium::Handler::Base {

    Osmium::OSMFile *m_outfile;

    Osmium::Output::OSM::Base *output;

public:

    ConvertHandler(Osmium::OSMFile *osmfile = new Osmium::OSMFile("-")) : m_outfile(osmfile) {
    }

    ~ConvertHandler() {
        delete m_outfile;
    }

    void callback_init() {
        output = m_outfile->create_output_file();
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
        {"input",                required_argument, 0, 'i'},
        {"output",               required_argument, 0, 'o'},
        {"help",                 no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    bool debug = false;

    const char *input = NULL;
    const char *input_type = NULL;

    const char *output = NULL;
    const char *output_type = NULL;

    while (1) {
        int c = getopt_long(argc, argv, "dhi:o:", long_options, 0);
        if (c == -1)
            break;

        switch (c) {
            case 'd':
                debug = true;
                break;
            case 'h':
                print_help();
                exit(0);
            case 'i':
                input_type = optarg;
                break;
            case 'o':
                output_type = optarg;
                break;
            default:
                exit(1);
        }
    }

    if (optind == argc-2) {
        input =  argv[optind];
        output = argv[optind+1];
    }
    else if (optind == argc-1) {
        input =  argv[optind];
        output = "-";
    }
    else {
        input =  "-";
        output = "-";
    }

    Osmium::Framework osmium(debug);

    Osmium::OSMFile infile(input);
    if (input_type) {
        infile.set_type_and_encoding(input_type);
    }

    Osmium::OSMFile* outfile = new Osmium::OSMFile(output);
    if (output_type) {
        outfile->set_type_and_encoding(output_type);
    }

    ConvertHandler *handler_convert = new ConvertHandler(outfile);

    infile.read<ConvertHandler>(handler_convert);

    delete handler_convert;
}

