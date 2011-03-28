#ifndef OSMIUM_XMLPARSER_HPP
#define OSMIUM_XMLPARSER_HPP

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sstream>

#include <expat.h>

namespace Osmium {

    class XMLParser {

        static const int buffer_size = 10240;

        OSM::Node     *node;
        OSM::Way      *way;
        OSM::Relation *relation;

        int fd; ///< The file descriptor we are reading the data from.
        struct callbacks *callbacks; ///< Functions to call for each object etc.

        osm_object_type_t last_object_type;

        Osmium::OSM::Object *current_object;

        static void XMLCALL start_element_wrapper(void *data, const XML_Char* element, const XML_Char** attrs) {
            ((Osmium::XMLParser *)data)->start_element(element, attrs);
        }

        static void XMLCALL end_element_wrapper(void *data, const XML_Char* element) {
            ((Osmium::XMLParser *)data)->end_element(element);
        }

      public:

        XMLParser(int in_fd, struct callbacks *cb) : fd(in_fd), callbacks(cb) {
        }

        ~XMLParser() {
        }

        void parse(Osmium::OSM::Node *in_node, Osmium::OSM::Way *in_way, Osmium::OSM::Relation *in_relation) {
            int done;
            current_object = 0;

            node     = in_node;
            way      = in_way;
            relation = in_relation;

            last_object_type = NODE;

            XML_Parser parser = XML_ParserCreate(0);
            if (!parser) {
                throw std::runtime_error("Error creating parser");
            }

            XML_SetUserData(parser, this);

            XML_SetElementHandler(parser, XMLParser::start_element_wrapper, XMLParser::end_element_wrapper);

            if (callbacks->before_nodes) { callbacks->before_nodes(); }

            do {
                void *buffer = XML_GetBuffer(parser, buffer_size);
                if (buffer == 0) {
                    throw std::runtime_error("out of memory");
                }

                int result = read(fd, buffer, buffer_size);
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

            if (callbacks->after_relations) { callbacks->after_relations(); }
        }

      private:

        void init_object(OSM::Object *obj, const XML_Char **attrs) {
            current_object = obj;
            current_object->reset();
            for (int count = 0; attrs[count]; count += 2) {
                current_object->set_attribute(attrs[count], attrs[count+1]);
            }
        }

        void start_element(const XML_Char* element, const XML_Char** attrs) {
            // order in the following "if" statements is based on frequency of tags in planet file
            if (!strcmp(element, "nd")) {
                for (int count = 0; attrs[count]; count += 2) {
                    if (!strcmp(attrs[count], "ref")) {
                        way->add_node(atoll(attrs[count+1]));
                    }
                }
            }
            else if (!strcmp(element, "node")) {
                if (last_object_type != NODE) {
                    throw std::runtime_error("OSM files must be ordered: first nodes, then ways, then relations. You can use Osmosis with the --sort option to do this.");
                }
                init_object(node, attrs);
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
                if (current_object) {
                    current_object->add_tag(key, value);
                }
            }
            else if (!strcmp(element, "way")) {
                if (last_object_type == NODE) {
                    if (callbacks->after_nodes) { callbacks->after_nodes(); }
                    last_object_type = WAY;
                    if (callbacks->before_ways) { callbacks->before_ways(); }
                }
                if (last_object_type == RELATION) {
                    throw std::runtime_error("OSM files must be ordered: first nodes, then ways, then relations. You can use Osmosis with the --sort option to do this.");
                }
                init_object(way, attrs);
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
                if (last_object_type == WAY) {
                    if (callbacks->after_ways) { callbacks->after_ways(); }
                    last_object_type = RELATION;
                    if (callbacks->before_relations) { callbacks->before_relations(); }
                }
                if (last_object_type == NODE) {
                    throw std::runtime_error("OSM files must be ordered: first nodes, then ways, then relations. You can use Osmosis with the --sort option to do this.");
                }
                init_object(relation, attrs);
            }
        }

        void end_element(const XML_Char* element) {
            if (!strcmp(element, "node")) {
                if (callbacks->node) { callbacks->node(node); }
                current_object = 0;
            }
            if (!strcmp(element, "way")) {
                if (callbacks->way) { callbacks->way(way); }
                current_object = 0;
            }
            if (!strcmp(element, "relation")) {
                if (callbacks->relation) { callbacks->relation(relation); }
                current_object = 0;
            }
        }

    }; // class XMLParser

} // namespace Osmium

#endif // OSMIUM_XMLPARSER_HPP
