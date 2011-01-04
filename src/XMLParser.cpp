
#include "osmium.hpp"
#include "XMLParser.hpp"

#include <unistd.h>
#include <string>
#include <cstring>
#include <sstream>

#define OBJECT (*((Osmium::OSM::Object **) data))

namespace Osmium {

    OSM::Node     *node;
    OSM::Way      *way;
    OSM::Relation *relation;

    struct callbacks *callbacks;

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
                if (callbacks->after_nodes) { callbacks->after_nodes(); }
                last_object_type = WAY;
                if (callbacks->before_ways) { callbacks->before_ways(); }
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
                if (callbacks->after_ways) { callbacks->after_ways(); }
                last_object_type = RELATION;
                if (callbacks->before_relations) { callbacks->before_relations(); }
            }
            init_object(data, relation, attrs);
        }
    }

    void XMLCALL XMLParser::endElement(void *data, const XML_Char* element) {
        if (!strcmp(element, "node")) {
            if (callbacks->node) { callbacks->node(*((Osmium::OSM::Node **) data)); }
            OBJECT = 0;
        }
        if (!strcmp(element, "way")) {
            if (callbacks->way) { callbacks->way(*((Osmium::OSM::Way **) data)); }
            OBJECT = 0;
        }
        if (!strcmp(element, "relation")) {
            if (callbacks->relation) { callbacks->relation(*((Osmium::OSM::Relation **) data)); }
            OBJECT = 0;
        }
    }

    void XMLParser::parse(int fd, struct callbacks *cb, Osmium::OSM::Node *in_node, Osmium::OSM::Way *in_way, Osmium::OSM::Relation *in_relation) {
        int done;
        Osmium::OSM::Object *current_object = 0;

        callbacks = cb;

        node     = in_node;
        way      = in_way;
        relation = in_relation;

        last_object_type = NODE;

        XML_Parser parser = XML_ParserCreate(0);
        if (!parser) {
            throw std::runtime_error("Error creating parser");
        }

        XML_SetUserData(parser, (void *) &current_object);

        XML_SetElementHandler(parser, XMLParser::startElement, XMLParser::endElement);

        if (callbacks->before_nodes) { callbacks->before_nodes(); }

        do {
            void *buffer = XML_GetBuffer(parser, OSMIUM_XMLPARSER_BUFFER_SIZE);
            if (buffer == 0) {
                throw std::runtime_error("out of memory");
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
                throw std::runtime_error(errorDesc.str());
            }
        } while (!done);

        XML_ParserFree(parser);
        error = "";

        if (callbacks->after_relations) { callbacks->after_relations(); }
    }
}

