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
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

// this is required to allow using libxml's xmlwriter in parallel to expat xml parser under debian
#undef XMLCALL
#include <libxml/xmlwriter.h>

#include <osmium/output.hpp>

// XXX error handling is mostly missing...

namespace Osmium {

    namespace Output {

        class XML : public Base {

        public:

            XML(Osmium::OSMFile& file) : Base(file), m_last_op('\0') {
                xml_writer = xmlNewTextWriter(xmlOutputBufferCreateFd(this->get_fd(), NULL));
            }

            ~XML() {
            }

            void init(Osmium::OSM::Meta& meta) {
                xmlTextWriterSetIndent(xml_writer, 1);
                xmlTextWriterSetIndentString(xml_writer, BAD_CAST "  ");
                xmlTextWriterStartDocument(xml_writer, NULL, "utf-8", NULL); // <?xml .. ?>

                if (m_file.get_type() == Osmium::OSMFile::FileType::Change()) {
                    xmlTextWriterStartElement(xml_writer, BAD_CAST "osmChange");  // <osmChange>
                } else {
                    xmlTextWriterStartElement(xml_writer, BAD_CAST "osm");  // <osm>
                }
                xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "version", BAD_CAST "0.6");
                xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "generator", BAD_CAST "Osmium (http://wiki.openstreetmap.org/wiki/Osmium)");
                if (meta.bounds().defined()) {
                    xmlTextWriterStartElement(xml_writer, BAD_CAST "bounds"); // <bounds>

                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "minlon", "%.7f", meta.bounds().bl().lon());
                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "minlat", "%.7f", meta.bounds().bl().lat());
                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "maxlon", "%.7f", meta.bounds().tr().lon());
                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "maxlat", "%.7f", meta.bounds().tr().lat());

                    xmlTextWriterEndElement(xml_writer); // </bounds>
                }
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                if (m_file.get_type() == Osmium::OSMFile::FileType::Change()) {
                    open_close_op_tag(node->visible() ? (node->version() == 1 ? 'c' : 'm') : 'd');
                }
                xmlTextWriterStartElement(xml_writer, BAD_CAST "node"); // <node>

                write_meta(node);

                const Osmium::OSM::Position position = node->position();
                xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "lat", "%.7f", position.lat());
                xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "lon", "%.7f", position.lon());

                write_tags(node->tags());

                xmlTextWriterEndElement(xml_writer); // </node>
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& way) {
                if (m_file.get_type() == Osmium::OSMFile::FileType::Change()) {
                    open_close_op_tag(way->visible() ? (way->version() == 1 ? 'c' : 'm') : 'd');
                }
                xmlTextWriterStartElement(xml_writer, BAD_CAST "way"); // <way>

                write_meta(way);

                Osmium::OSM::WayNodeList::const_iterator end = way->nodes().end();
                for (Osmium::OSM::WayNodeList::const_iterator it = way->nodes().begin(); it != end; ++it) {
                    xmlTextWriterStartElement(xml_writer, BAD_CAST "nd"); // <nd>
                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "ref", "%d", it->ref());
                    xmlTextWriterEndElement(xml_writer); // </nd>
                }

                write_tags(way->tags());

                xmlTextWriterEndElement(xml_writer); // </way>
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                if (m_file.get_type() == Osmium::OSMFile::FileType::Change()) {
                    open_close_op_tag(relation->visible() ? (relation->version() == 1 ? 'c' : 'm') : 'd');
                }
                xmlTextWriterStartElement(xml_writer, BAD_CAST "relation"); // <relation>

                write_meta(relation);

                Osmium::OSM::RelationMemberList::const_iterator end = relation->members().end();
                for (Osmium::OSM::RelationMemberList::const_iterator it = relation->members().begin(); it != end; ++it) {
                    xmlTextWriterStartElement(xml_writer, BAD_CAST "member"); // <member>

                    xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "type", BAD_CAST it->type_name());
                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "ref", "%d", it->ref());
                    xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "role", BAD_CAST it->role());

                    xmlTextWriterEndElement(xml_writer); // </member>
                }

                write_tags(relation->tags());

                xmlTextWriterEndElement(xml_writer); // </relation>
            }

            void final() {
                if (m_file.get_type() == Osmium::OSMFile::FileType::Change()) {
                    open_close_op_tag('\0');
                }
                xmlTextWriterEndElement(xml_writer); // </osm> or </osmChange>
                xmlFreeTextWriter(xml_writer);
                m_file.close();
            }

        private:

            xmlTextWriterPtr xml_writer;

            void write_meta(const shared_ptr<Osmium::OSM::Object const>& object) {
                xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "id",      "%d", object->id());
                if (object->version()) {
                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "version", "%d", object->version());
                }
                if (object->timestamp()) {
                    xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "timestamp", BAD_CAST object->timestamp_as_string().c_str());
                }

                // uid <= 0 -> anonymous
                if (object->uid() > 0) {
                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "uid", "%d", object->uid());
                    xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "user", BAD_CAST object->user());
                }

                if (object->changeset()) {
                    xmlTextWriterWriteFormatAttribute(xml_writer, BAD_CAST "changeset", "%d", object->changeset());
                }

                if (m_file.has_multiple_object_versions() && m_file.get_type() != Osmium::OSMFile::FileType::Change()) {
                    xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "visible", object->visible() ? BAD_CAST "true" : BAD_CAST "false");
                }
            }

            void write_tags(const Osmium::OSM::TagList& tags) {
                Osmium::OSM::TagList::const_iterator end = tags.end();
                for (Osmium::OSM::TagList::const_iterator it = tags.begin(); it != end; ++it) {
                    xmlTextWriterStartElement(xml_writer, BAD_CAST "tag"); // <tag>
                    xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "k", BAD_CAST it->key());
                    xmlTextWriterWriteAttribute(xml_writer, BAD_CAST "v", BAD_CAST it->value());
                    xmlTextWriterEndElement(xml_writer); // </tag>
                }
            }

            char m_last_op;

            void open_close_op_tag(char op) {
                if (op == m_last_op) {
                    return;
                }

                if (m_last_op) {
                    xmlTextWriterEndElement(xml_writer);
                }

                switch (op) {
                    case 'c':
                        xmlTextWriterStartElement(xml_writer, BAD_CAST "create");
                        break;
                    case 'm':
                        xmlTextWriterStartElement(xml_writer, BAD_CAST "modify");
                        break;
                    case 'd':
                        xmlTextWriterStartElement(xml_writer, BAD_CAST "delete");
                        break;
                }

                m_last_op = op;
            }

        }; // class XML

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_XML_HPP
