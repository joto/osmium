#ifndef OSMIUM_OUTPUT_XML_HPP
#define OSMIUM_OUTPUT_XML_HPP

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

#include <fstream>

// this is required to allow using libxml's xmlwriter in paralell to expat xml parser under debian
#undef XMLCALL
#include <libxml/xmlwriter.h>

#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>

namespace Osmium {

    namespace Output {

        class XML {

        protected:

            xmlTextWriterPtr w;

            void writeMeta(Osmium::OSM::Object *e) {
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "id", "%d", e->id);
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "version", "%d", e->version);
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "changeset", "%d", e->changeset);
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "timestamp", "%s", e->get_timestamp_str());
                
                // uid == 0 -> anonymous
                if(e->uid > 0) {
                    xmlTextWriterWriteFormatAttribute(w, BAD_CAST "uid", "%d", e->uid);
                    xmlTextWriterWriteFormatAttribute(w, BAD_CAST "user", "%s", e->user);
                }
                
                if(writeVisibleAttr) {
                    if(e->visible)
                        xmlTextWriterWriteAttribute(w, BAD_CAST "visible", BAD_CAST "true");
                    else
                        xmlTextWriterWriteAttribute(w, BAD_CAST "visible", BAD_CAST "false");
                
                }
            }

            void writeTags(Osmium::OSM::Object *e) {
                for(int i=0, l = e->tag_count(); i<l; i++) {
                    xmlTextWriterStartElement(w, BAD_CAST "tag"); // <tag>
                    xmlTextWriterWriteFormatAttribute(w, BAD_CAST "k", "%s", e->get_tag_key(i));
                    xmlTextWriterWriteFormatAttribute(w, BAD_CAST "v", "%s", e->get_tag_value(i));
                    xmlTextWriterEndElement(w); // </tag>
                }
            }

        public:

            bool writeVisibleAttr;

            //v8::Persistent<v8::Object> js_object;

            XML(const char *filename) {
                writeVisibleAttr = false;
                
                w = xmlNewTextWriterFilename(filename, 0);
                xmlTextWriterSetIndent(w, 1);
                xmlTextWriterStartDocument(w, NULL, NULL, NULL); // <?xml .. ?>
                
                xmlTextWriterStartElement(w, BAD_CAST "osm");  // <osm>
                xmlTextWriterWriteAttribute(w, BAD_CAST "version", BAD_CAST "0.6");
                xmlTextWriterWriteAttribute(w, BAD_CAST "generator", BAD_CAST "MaZderMind's History Splitter <https://github.com/MaZderMind/OpenStreetMap-History-API/tree/master/splitter>");
                
                //js_object = v8::Persistent<v8::Object>::New( Osmium::Javascript::Template::create_output_xml_instance(this) );
            }

            ~XML() {
                xmlTextWriterEndElement(w); // </osm>
                xmlFreeTextWriter(w);
                w = NULL;
            }

            void writeBounds(double minlat, double minlon, double maxlat, double maxlon) {
                xmlTextWriterStartElement(w, BAD_CAST "bounds"); // <bounds>
                
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "minlat", "%f", minlat);
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "minlon", "%f", minlon);
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "maxlat", "%f", maxlat);
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "maxlon", "%f", maxlon);
                
                xmlTextWriterEndElement(w); // </bounds>
            }

            void write(Osmium::OSM::Node* e) {
                xmlTextWriterStartElement(w, BAD_CAST "node"); // <node>
                
                this->writeMeta(e);
                
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "lat", "%f", e->get_lat());
                xmlTextWriterWriteFormatAttribute(w, BAD_CAST "lon", "%f", e->get_lon());
                
                this->writeTags(e);
                
                xmlTextWriterEndElement(w); // </node>
            }
            
            void write(Osmium::OSM::Way* e) {
                xmlTextWriterStartElement(w, BAD_CAST "way"); // <way>
                
                this->writeMeta(e);
                
                for(int i=0, l=e->node_count(); i<l; i++) {
                    xmlTextWriterStartElement(w, BAD_CAST "nd"); // <nd>
                    xmlTextWriterWriteFormatAttribute(w, BAD_CAST "ref", "%d", e->nodes[i]);
                    xmlTextWriterEndElement(w); // </nd>
                }
                
                this->writeTags(e);
                
                xmlTextWriterEndElement(w); // </way>
            }
            
            void write(Osmium::OSM::Relation* e) {
                xmlTextWriterStartElement(w, BAD_CAST "relation"); // <relation>
                
                this->writeMeta(e);
                
                const Osmium::OSM::RelationMember *mem;
                for(int i=0, l=e->member_count(); i<l; i++) {
                    mem = e->get_member(i);
                    
                    xmlTextWriterStartElement(w, BAD_CAST "member"); // <member>
                    
                    switch(mem->get_type()) {
                        case 'n':
                            xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "node");
                            break;
                        
                        case 'w':
                            xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "way");
                            break;
                        
                        case 'r':
                            xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "relation");
                            break;
                    }
                    xmlTextWriterWriteFormatAttribute(w, BAD_CAST "ref", "%d", mem->get_ref());
                    xmlTextWriterWriteFormatAttribute(w, BAD_CAST "role", "%s", mem->get_role());
                    xmlTextWriterEndElement(w); // </member>
                }
                
                this->writeTags(e);
                
                xmlTextWriterEndElement(w); // </relation>
            }

            //v8::Handle<v8::Object> get_js_object() const {
            //    return js_object;
            //}

            //v8::Handle<v8::Value> js_print(const v8::Arguments& args) {
            //    for (int i = 0; i < args.Length(); i++) {
            //        if (i != 0) {
            //            out << '\t';
            //        }
            //        v8_String_to_ostream(args[i]->ToString(), out);
            //    }
            //    out << std::endl;
            //    return v8::Integer::New(1);
            //}

            //v8::Handle<v8::Value> js_close(const v8::Arguments& /*args*/) {
            //    out.flush();
            //    out.close();
            //    return v8::Undefined();
            //}

        }; // class XML

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_XML_HPP
