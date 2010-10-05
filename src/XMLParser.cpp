
#include "osmium.hpp"
#include "XMLParser.hpp"

#include <unistd.h>
#include <string>
#include <cstring>
#include <sstream>

#define OBJECT (*((Osmium::OSM::Object **) data))

namespace Osmium {

    extern void init_handler();
    extern void before_nodes_handler();
    extern void after_nodes_handler();
    extern void before_ways_handler();
    extern void after_ways_handler();
    extern void before_relations_handler();
    extern void after_relations_handler();
    extern void final_handler();
    extern void object_handler(OSM::Object *object);

    OSM::Node     *node;
    OSM::Way      *way;
    OSM::Relation *relation;

    osm_object_type_t last_object_type;

    std::string XMLParser::error = "";

    void init_object(void *data, OSM::Object *obj, const XML_Char **attrs) {
        OBJECT = obj;
        OBJECT->reset();
        for (int count = 0; attrs[count]; count += 2) {
            OBJECT->set_attribute(attrs[count], attrs[count+1]);
        }
    }

    void XMLCALL XMLParser::startElement(void *data, const XML_Char* element, const XML_Char** attrs) {
        // order in the following "if" statements is based on frequency of tags in planet file
        if (!strcmp(element, "nd")) {
            for (int count = 0; attrs[count]; count += 2) {
                if (!strcmp(attrs[count], "ref")) {
                    way->add_node(atoll(attrs[count+1]));
                }
            }
        }
        else if (!strcmp(element, "node")) {
            init_object(data, node, attrs);
        }
        else if (!strcmp(element, "tag")) {
            const char *key = "", *value = "";
            for (int count = 0; attrs[count]; count += 2) {
                if (attrs[count][0] == 'k' && attrs[count][1] == 0) {
                    key = attrs[count+1];
                }
                if (attrs[count][0] == 'v' && attrs[count][1] == 0) {
                    value = attrs[count+1];
                }
            }
            // XXX assert key, value exist
            if (OBJECT) {
                OBJECT->add_tag(key, value);
            }
        }
        else if (!strcmp(element, "way")) {
            if (last_object_type != WAY) {
                after_nodes_handler();
                last_object_type = WAY;
                before_ways_handler();
            }
            init_object(data, way, attrs);
        }
        else if (!strcmp(element, "member")) {
            char        type = 'x';
            uint64_t    ref  = 0;
            const char *role = "";
            for (int count = 0; attrs[count]; count += 2) {
                if (!strcmp(attrs[count], "type")) {
                    type = (char)attrs[count+1][0];
                }
                else if (!strcmp(attrs[count], "ref")) {
                    ref = atoll(attrs[count+1]);
                }
                else if (!strcmp(attrs[count], "role")) {
                    role = (char *)attrs[count+1];
                }
            }
            // XXX assert type, ref, role are set
            relation->add_member(type, ref, role);
        }
        else if (!strcmp(element, "relation")) {
            if (last_object_type != RELATION) {
                after_ways_handler();
                last_object_type = RELATION;
                before_relations_handler();
            }
            init_object(data, relation, attrs);
        }
    }

    void XMLCALL XMLParser::endElement(void *data, const XML_Char* element) {
        if (!strcmp(element, "node") || !strcmp(element, "way") || !strcmp(element, "relation")) {
            Osmium::object_handler(OBJECT);
            OBJECT = 0;
        }
    }

    bool XMLParser::parse(int fd, Osmium::OSM::Node *in_node, Osmium::OSM::Way *in_way, Osmium::OSM::Relation *in_relation) {
        int done;
        Osmium::OSM::Object *current_object = 0;

        node     = in_node;
        way      = in_way;
        relation = in_relation;

        last_object_type = NODE;

        XML_Parser parser = XML_ParserCreate(0);
        if (!parser) {
            error = "Error creating parser";
            return false;
        }

        XML_SetUserData(parser, (void *) &current_object);

        XML_SetElementHandler(parser, XMLParser::startElement, XMLParser::endElement);

        init_handler();
        before_nodes_handler();

        do {
            void *buffer = XML_GetBuffer(parser, OSMIUM_XMLPARSER_BUFFER_SIZE);
            if (buffer == 0) {
                error = "out of memory";
                return false;
            }

            int result = read(fd, buffer, OSMIUM_XMLPARSER_BUFFER_SIZE);
            if (result < 0) {
                exit(1);
            }
            done = (result == 0);
            if (XML_ParseBuffer(parser, result, done) == XML_STATUS_ERROR)
            {
                XML_Error errorCode = XML_GetErrorCode(parser);
                long errorLine = XML_GetCurrentLineNumber(parser);
                long errorCol = XML_GetCurrentColumnNumber(parser);
                const XML_LChar *errorString = XML_ErrorString(errorCode);

                std::stringstream errorDesc;
                errorDesc << "XML parsing error at line " << errorLine << ":" << errorCol;
                errorDesc << ": " << errorString;
                error = errorDesc.str();
                return false;
            }
        } while (!done);

        XML_ParserFree(parser);
        error = "";

        after_relations_handler();
        final_handler();

        return true;
    }
}

