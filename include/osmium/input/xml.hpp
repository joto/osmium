#ifndef OSMIUM_INPUT_XML_HPP
#define OSMIUM_INPUT_XML_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#define OSMIUM_LINK_WITH_LIBS_EXPAT -lexpat

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <expat.h>

#include <osmium/input.hpp>

namespace Osmium {

    namespace Input {

        /**
        * Class for parsing OSM XML files.
        *
        * Generally you are not supposed to instantiate this class yourself.
        * Use the Osmium::Input::read() function instead.
        *
        * @tparam THandler A handler class (subclass of Osmium::Handler::Base).
        */
        template <class THandler>
        class XML : public Base<THandler> {

        public:

            /**
            * Instantiate XML Parser.
            *
            * @param file OSMFile instance.
            * @param handler Instance of THandler.
            */
            XML(const Osmium::OSMFile& file, THandler& handler) :
                Base<THandler>(file, handler),
                m_current_object(NULL),
                m_context(context_root),
                m_last_context(context_root),
                m_in_delete_section(false) {
            }

            void parse() {
                XML_Parser parser = XML_ParserCreate(0);
                if (!parser) {
                    throw std::runtime_error("Error creating parser");
                }

                XML_SetUserData(parser, this);

                XML_SetElementHandler(parser, Osmium::Input::XML<THandler>::start_element_wrapper, Osmium::Input::XML<THandler>::end_element_wrapper);

                try {
                    int done;
                    do {
                        void* buffer = XML_GetBuffer(parser, c_buffer_size);
                        if (buffer == 0) {
                            throw std::runtime_error("out of memory");
                        }

                        int result = ::read(this->fd(), buffer, c_buffer_size);
                        if (result < 0) {
                            throw std::runtime_error("read error");
                        }
                        done = (result == 0);
                        if (XML_ParseBuffer(parser, result, done) == XML_STATUS_ERROR) {
                            XML_Error errorCode = XML_GetErrorCode(parser);
                            long errorLine = XML_GetCurrentLineNumber(parser);
                            long errorCol = XML_GetCurrentColumnNumber(parser);
                            const XML_LChar* errorString = XML_ErrorString(errorCode);

                            std::stringstream errorDesc;
                            errorDesc << "XML parsing error at line " << errorLine << ":" << errorCol;
                            errorDesc << ": " << errorString;
                            throw std::runtime_error(errorDesc.str());
                        }
                    } while (!done);
                    XML_ParserFree(parser);

                    this->call_after_and_before_on_handler(UNKNOWN);
                } catch (Osmium::Handler::StopReading) {
                    // if a handler says to stop reading, we do
                }
                this->call_final_on_handler();
            }

        private:

            static const int c_buffer_size = 10240;

            Osmium::OSM::Object* m_current_object;

            enum context_t {
                context_root,
                context_top,
                context_node,
                context_way,
                context_relation,
                context_in_object
            };

            context_t m_context;
            context_t m_last_context;

            /**
             * This is used only for change files which contain create, modify,
             * and delete sections.
             */
            bool m_in_delete_section;

            static void XMLCALL start_element_wrapper(void* data, const XML_Char* element, const XML_Char** attrs) {
                static_cast<Osmium::Input::XML<THandler> *>(data)->start_element(element, attrs);
            }

            static void XMLCALL end_element_wrapper(void* data, const XML_Char* element) {
                static_cast<Osmium::Input::XML<THandler> *>(data)->end_element(element);
            }

            void init_object(Osmium::OSM::Object& obj, const XML_Char** attrs) {
                if (m_in_delete_section) {
                    obj.visible(false);
                }
                m_current_object = &obj;
                for (int count = 0; attrs[count]; count += 2) {
                    if (!strcmp(attrs[count], "lon")) {
                        if (this->m_node) {
                            this->m_node->lon(atof(attrs[count+1]));
                        }
                    } else if (!strcmp(attrs[count], "lat")) {
                        if (this->m_node) {
                            this->m_node->lat(atof(attrs[count+1]));
                        }
                    } else {
                        m_current_object->set_attribute(attrs[count], attrs[count+1]);
                    }
                }
            }

            void check_tag(const XML_Char* element, const XML_Char** attrs) {
                if (!strcmp(element, "tag")) {
                    const char* key = "";
                    const char* value = "";
                    for (int count = 0; attrs[count]; count += 2) {
                        if (attrs[count][0] == 'k' && attrs[count][1] == 0) {
                            key = attrs[count+1];
                        }
                        if (attrs[count][0] == 'v' && attrs[count][1] == 0) {
                            value = attrs[count+1];
                        }
                    }
                    m_current_object->tags().add(key, value);
                }
            }

            void start_element(const XML_Char* element, const XML_Char** attrs) {
                switch (m_context) {
                    case context_root:
                        if (!strcmp(element, "osm") || !strcmp(element, "osmChange")) {
                            for (int count = 0; attrs[count]; count += 2) {
                                if (!strcmp(attrs[count], "version")) {
                                    if (strcmp(attrs[count+1], "0.6")) {
                                        throw std::runtime_error("can only read version 0.6 files");
                                    }
                                } else if (!strcmp(attrs[count], "generator")) {
                                    this->meta().generator(attrs[count+1]);
                                }
                            }
                        }
                        m_context = context_top;
                        break;
                    case context_top:
                        if (!strcmp(element, "node")) {
                            this->call_after_and_before_on_handler(NODE);
                            init_object(this->prepare_node(), attrs);
                            m_context = context_node;
                        } else if (!strcmp(element, "way")) {
                            this->call_after_and_before_on_handler(WAY);
                            init_object(this->prepare_way(), attrs);
                            m_context = context_way;
                        } else if (!strcmp(element, "relation")) {
                            this->call_after_and_before_on_handler(RELATION);
                            init_object(this->prepare_relation(), attrs);
                            m_context = context_relation;
                        } else if (!strcmp(element, "bounds")) {
                            Osmium::OSM::Position min;
                            Osmium::OSM::Position max;
                            for (int count = 0; attrs[count]; count += 2) {
                                if (!strcmp(attrs[count], "minlon")) {
                                    min.lon(atof(attrs[count+1]));
                                } else if (!strcmp(attrs[count], "minlat")) {
                                    min.lat(atof(attrs[count+1]));
                                } else if (!strcmp(attrs[count], "maxlon")) {
                                    max.lon(atof(attrs[count+1]));
                                } else if (!strcmp(attrs[count], "maxlat")) {
                                    max.lat(atof(attrs[count+1]));
                                }
                            }
                            this->meta().bounds().extend(min).extend(max);
                        } else if (!strcmp(element, "delete")) {
                            m_in_delete_section = true;
                        }
                        break;
                    case context_node:
                        m_last_context = context_node;
                        m_context = context_in_object;
                        check_tag(element, attrs);
                        break;
                    case context_way:
                        m_last_context = context_way;
                        m_context = context_in_object;
                        if (!strcmp(element, "nd")) {
                            for (int count = 0; attrs[count]; count += 2) {
                                if (!strcmp(attrs[count], "ref")) {
                                    this->m_way->add_node(Osmium::string_to_osm_object_id_t(attrs[count+1]));
                                }
                            }
                        } else {
                            check_tag(element, attrs);
                        }
                        break;
                    case context_relation:
                        m_last_context = context_relation;
                        m_context = context_in_object;
                        if (!strcmp(element, "member")) {
                            char        type = 'x';
                            uint64_t    ref  = 0;
                            const char* role = "";
                            for (int count = 0; attrs[count]; count += 2) {
                                if (!strcmp(attrs[count], "type")) {
                                    type = static_cast<char>(attrs[count+1][0]);
                                } else if (!strcmp(attrs[count], "ref")) {
                                    ref = Osmium::string_to_osm_object_id_t(attrs[count+1]);
                                } else if (!strcmp(attrs[count], "role")) {
                                    role = static_cast<const char*>(attrs[count+1]);
                                }
                            }
                            // XXX assert type, ref, role are set
                            if (m_current_object && this->m_relation) {
                                this->m_relation->add_member(type, ref, role);
                            }
                        } else {
                            check_tag(element, attrs);
                        }
                        break;
                    case context_in_object:
                        // fallthrough
                    default:
                        assert(false); // should never be here
                }
            }

            void end_element(const XML_Char* element) {
                switch (m_context) {
                    case context_root:
                        assert(false); // should never be here
                        break;
                    case context_top:
                        if (!strcmp(element, "osm") || !strcmp(element, "osmChange")) {
                            m_context = context_root;
                        } else if (!strcmp(element, "delete")) {
                            m_in_delete_section = false;
                        }
                        break;
                    case context_node:
                        this->call_node_on_handler();
                        m_current_object = NULL;
                        m_context = context_top;
                        break;
                    case context_way:
                        this->call_way_on_handler();
                        m_current_object = NULL;
                        m_context = context_top;
                        break;
                    case context_relation:
                        this->call_relation_on_handler();
                        m_current_object = NULL;
                        m_context = context_top;
                        break;
                    case context_in_object:
                        m_context = m_last_context;
                        break;
                    default:
                        assert(false); // should never be here
                }
            }

        }; // class XML

    } // namespace Input

} // namespace Osmium

#endif // OSMIUM_INPUT_XML_HPP
