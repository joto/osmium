 #include <cstdlib>
#include <getopt.h>

#define OSMIUM_MAIN
#include <osmium.hpp>

int main(int argc, char *argv[]) {
    const int nodes =      1000;
    const int ways =        500;
    const int relations =   500;
    
    const int versions =     40;
    const int members =     200;
    const int tags =          3;
    
    const char *keys[] = {
        "highway",
        "addr:housenumber",
        "addr:street",
        "amenity",
        "cuisine",
        "name",
        "opening_hours",
        "operator",
        "phone",
        "smoking"
    };
    const char *vals[] = {
        "road",
        "49",
        "Alzeyer Straße",
        "restaurant",
        "pizza",
        "Zur Scheune",
        "Tu-Su 11:30-14:00,17:00-22:30",
        "Gurnaib Singh",
        "+496733929569",
        "yes"
    };
    const int numtags = sizeof(keys) / sizeof(keys[0]);
    int lasttag = 0;
    
    if(argc != 2) {
        fprintf(stderr, "no output filename\n");
        fprintf(stderr, "Usage: %s [output]\n", argv[0]);
        exit(1);
    }
    
    std::string filename = std::string(argv[1]);
    
    Osmium::Framework osmium(false);
    Osmium::Output::OSM::Base *output = Osmium::Output::OSM::create(filename);
    output->write_init();
    
    unsigned long int item = 0;
    Osmium::OSM::Node *node = new Osmium::OSM::Node();
    for(int id = 1; id<=nodes; id++) {
        for(int version = 1; version<=versions; version++) {
            node->reset();
            node->set_id(id);
            node->set_version(version);
            for(int tag = 0; tag<tags; tag++) {
                node->add_tag(keys[lasttag], vals[lasttag]);
                if(++lasttag > numtags) lasttag = 0;
            }
            
            fprintf(stderr, "item %lu\n", ++item);
            output->write(node);
        }
    }

    Osmium::OSM::Way *way = new Osmium::OSM::Way();
    for(int id = 1; id<=ways; id++) {
        for(int version = 1; version<=versions; version++) {
            way->reset();
            way->set_id(id);
            way->set_version(version);
            for(int tag = 0; tag<tags; tag++) {
                way->add_tag(keys[tag], vals[tag]);
                if(++tag > numtags) tag = 0;
            }
            for(int member = 1; member<=members; member++) {
                way->add_node(member);
            }
            
            fprintf(stderr, "item %lu\n", ++item);
            output->write(way);
        }
    }

    output->write_final();
    delete output;
}

