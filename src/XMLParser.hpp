#ifndef OSMIUM_XMLPARSER_HPP
#define OSMIUM_XMLPARSER_HPP

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <expat.h>

#define OSMIUM_XMLPARSER_BUFFER_SIZE 10240

namespace Osmium {

    class XMLParser {

        static void startElement(void *data, const XML_Char* element, const XML_Char** attrs);
        static void   endElement(void *data, const XML_Char* element);

    public:

        static std::string error;

        static void parse(int fd, struct callbacks *callbacks, Osmium::OSM::Node *in_node, Osmium::OSM::Way *in_way, Osmium::OSM::Relation *in_relation);
        static std::string getError() {
            return error;
        }

    }; // class XMLParser

} // namespace Osmium

#endif // OSMIUM_XMLPARSER_HPP
