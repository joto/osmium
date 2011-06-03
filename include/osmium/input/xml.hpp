#ifndef OSMIUM_XMLPARSER_HPP
#define OSMIUM_XMLPARSER_HPP

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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sstream>

#include <expat.h>

namespace Osmium {

    namespace Input {

        template <class THandler>
        class XML : public Base<THandler> {

            static const int buffer_size = 10240;

            int fd; ///< The file descriptor we are reading the data from.

            Osmium::OSM::Object *current_object;

            static void XMLCALL start_element_wrapper(void *data, const XML_Char* element, const XML_Char** attrs) {
                ((Osmium::Input::XML<THandler> *)data)->start_element(element, attrs);
            }

            static void XMLCALL end_element_wrapper(void *data, const XML_Char* element) {
                ((Osmium::Input::XML<THandler> *)data)->end_element(element);
            }

        public:

            /**
            * Instantiate XML Parser
            *
            * @param in_fd File descripter to read data from.
            * @param h Instance of THandler or NULL.
            */
            XML(int in_fd, THandler *h) __attribute__((noinline)) : Base<THandler>(h), fd(in_fd) {
            }

            void parse() __attribute__((noinline)) {
                int done;
                current_object = 0;

                XML_Parser parser = XML_ParserCreate(0);
                if (!parser) {
                    throw std::runtime_error("Error creating parser");
                }

                XML_SetUserData(parser, this);

                XML_SetElementHandler(parser, Osmium::Input::XML<THandler>::start_element_wrapper, Osmium::Input::XML<THandler>::end_element_wrapper);

                try {
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
                        if (XML_ParseBuffer(parser, result, done) == XML_STATUS_ERROR) {
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
                } catch (Osmium::Input::StopReading) {
                    // if a handler says to stop reading, we do
                }

                XML_ParserFree(parser);

                this->call_after_and_before_handlers(UNKNOWN);
            }

        private:

            void init_object(OSM::Object *obj, const XML_Char **attrs) {
                current_object = obj;
                current_object->reset();
                for (int count = 0; attrs[count]; count += 2) {
                    if (!strcmp(attrs[count], "lon")) {
                        dynamic_cast<Osmium::OSM::Node *>(current_object)->set_x(atof(attrs[count+1]));
                    } else if (!strcmp(attrs[count], "lat")) {
                        dynamic_cast<Osmium::OSM::Node *>(current_object)->set_y(atof(attrs[count+1]));
                    } else {
                        current_object->set_attribute(attrs[count], attrs[count+1]);
                    }
                }
            }

            void start_element(const XML_Char* element, const XML_Char** attrs) {
                // order in the following "if" statements is based on frequency of tags in planet file
                if (!strcmp(element, "nd")) {
                    for (int count = 0; attrs[count]; count += 2) {
                        if (!strcmp(attrs[count], "ref")) {
                            this->way->add_node(atoll(attrs[count+1]));
                        }
                    }
                } else if (!strcmp(element, "node")) {
                    this->call_after_and_before_handlers(NODE);
                    init_object(this->node, attrs);
                } else if (!strcmp(element, "tag")) {
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
                } else if (!strcmp(element, "way")) {
                    this->call_after_and_before_handlers(WAY);
                    init_object(this->way, attrs);
                } else if (!strcmp(element, "member")) {
                    char        type = 'x';
                    uint64_t    ref  = 0;
                    const char *role = "";
                    for (int count = 0; attrs[count]; count += 2) {
                        if (!strcmp(attrs[count], "type")) {
                            type = (char)attrs[count+1][0];
                        } else if (!strcmp(attrs[count], "ref")) {
                            ref = atoll(attrs[count+1]);
                        } else if (!strcmp(attrs[count], "role")) {
                            role = (char *)attrs[count+1];
                        }
                    }
                    // XXX assert type, ref, role are set
                    this->relation->add_member(type, ref, role);
                } else if (!strcmp(element, "relation")) {
                    this->call_after_and_before_handlers(RELATION);
                    init_object(this->relation, attrs);
                }
            }

            void end_element(const XML_Char* element) {
                if (!strcmp(element, "node")) {
                    this->handler->callback_node(this->node);
                    current_object = 0;
                }
                if (!strcmp(element, "way")) {
                    this->handler->callback_way(this->way);
                    current_object = 0;
                }
                if (!strcmp(element, "relation")) {
                    this->handler->callback_relation(this->relation);
                    current_object = 0;
                }
            }

        }; // class XML

    } // namespace Input

} // namespace Osmium

#endif // OSMIUM_XMLPARSER_HPP
