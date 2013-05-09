#ifndef OSMIUM_OUTPUT_XML_HPP
#define OSMIUM_OUTPUT_XML_HPP

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

#define OSMIUM_COMPILE_WITH_CFLAGS_XML2 `xml2-config --cflags`
#define OSMIUM_LINK_WITH_LIBS_XML2 `xml2-config --libs`

// this is required to allow using libxml's xmlwriter in parallel to expat xml parser under debian
#undef XMLCALL
#include <libxml/xmlwriter.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <osmium/output.hpp>

namespace Osmium {

    namespace Output {

        struct XMLWriteError {};

        namespace {

            void check_for_error(int count) {
                if (count < 0) {
                    throw XMLWriteError();
                }
            }
        }

        class XML : public Base {

            // objects of this class can't be copied
            XML(const XML&);
            XML& operator=(const XML&);

        public:

            XML(const Osmium::OSMFile& file) :
                Base(file),
                m_xml_output_buffer(xmlOutputBufferCreateFd(this->fd(), NULL)),
                m_xml_writer(xmlNewTextWriter(m_xml_output_buffer)),
                m_last_op('\0') {
                if (!m_xml_output_buffer || !m_xml_writer) {
                    throw XMLWriteError();
                }
            }

            void init(Osmium::OSM::Meta& meta) {
                check_for_error(xmlTextWriterSetIndent(m_xml_writer, 1));
                check_for_error(xmlTextWriterSetIndentString(m_xml_writer, BAD_CAST "  "));
                check_for_error(xmlTextWriterStartDocument(m_xml_writer, NULL, NULL, NULL)); // <?xml .. ?>

                if (m_file.type() == Osmium::OSMFile::FileType::Change()) {
                    check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "osmChange"));  // <osmChange>
                } else {
                    check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "osm"));  // <osm>
                }
                check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "version", BAD_CAST "0.6"));
                check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "generator", BAD_CAST m_generator.c_str()));
                if (meta.bounds().defined()) {
                    check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "bounds")); // <bounds>

                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "minlon", "%.7f", meta.bounds().bottom_left().lon()));
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "minlat", "%.7f", meta.bounds().bottom_left().lat()));
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "maxlon", "%.7f", meta.bounds().top_right().lon()));
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "maxlat", "%.7f", meta.bounds().top_right().lat()));

                    check_for_error(xmlTextWriterEndElement(m_xml_writer)); // </bounds>
                }
            }

            void node(const shared_ptr<Osmium::OSM::Node const>& node) {
                if (m_file.type() == Osmium::OSMFile::FileType::Change()) {
                    open_close_op_tag(node->visible() ? (node->version() == 1 ? 'c' : 'm') : 'd');
                }
                check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "node")); // <node>

                write_meta(node);

                if (node->position().defined()) {
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "lat", "%.7f", node->position().lat()));
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "lon", "%.7f", node->position().lon()));
                }

                write_tags(node->tags());

                check_for_error(xmlTextWriterEndElement(m_xml_writer)); // </node>
            }

            void way(const shared_ptr<Osmium::OSM::Way const>& way) {
                if (m_file.type() == Osmium::OSMFile::FileType::Change()) {
                    open_close_op_tag(way->visible() ? (way->version() == 1 ? 'c' : 'm') : 'd');
                }
                check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "way")); // <way>

                write_meta(way);

                Osmium::OSM::WayNodeList::const_iterator end = way->nodes().end();
                for (Osmium::OSM::WayNodeList::const_iterator it = way->nodes().begin(); it != end; ++it) {
                    check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "nd")); // <nd>
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "ref", "%" PRId64, it->ref()));
                    check_for_error(xmlTextWriterEndElement(m_xml_writer)); // </nd>
                }

                write_tags(way->tags());

                check_for_error(xmlTextWriterEndElement(m_xml_writer)); // </way>
            }

            void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
                if (m_file.type() == Osmium::OSMFile::FileType::Change()) {
                    open_close_op_tag(relation->visible() ? (relation->version() == 1 ? 'c' : 'm') : 'd');
                }
                check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "relation")); // <relation>

                write_meta(relation);

                Osmium::OSM::RelationMemberList::const_iterator end = relation->members().end();
                for (Osmium::OSM::RelationMemberList::const_iterator it = relation->members().begin(); it != end; ++it) {
                    check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "member")); // <member>

                    check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "type", BAD_CAST it->type_name()));
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "ref", "%" PRId64, it->ref()));
                    check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "role", BAD_CAST it->role()));

                    check_for_error(xmlTextWriterEndElement(m_xml_writer)); // </member>
                }

                write_tags(relation->tags());

                check_for_error(xmlTextWriterEndElement(m_xml_writer)); // </relation>
            }

            void final() {
                if (m_file.type() == Osmium::OSMFile::FileType::Change()) {
                    open_close_op_tag('\0');
                }
                check_for_error(xmlTextWriterEndElement(m_xml_writer)); // </osm> or </osmChange>
                xmlFreeTextWriter(m_xml_writer);
                m_file.close();
            }

        private:

            xmlOutputBufferPtr m_xml_output_buffer;
            xmlTextWriterPtr m_xml_writer;
            char m_last_op;

            void write_meta(const shared_ptr<Osmium::OSM::Object const>& object) {
                check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "id", "%" PRId64, object->id()));
                if (object->version()) {
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "version", "%d", object->version()));
                }
                if (object->timestamp()) {
                    check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "timestamp", BAD_CAST object->timestamp_as_string().c_str()));
                }

                // uid <= 0 -> anonymous
                if (object->uid() > 0) {
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "uid", "%d", object->uid()));
                    check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "user", BAD_CAST object->user()));
                }

                if (object->changeset()) {
                    check_for_error(xmlTextWriterWriteFormatAttribute(m_xml_writer, BAD_CAST "changeset", "%d", object->changeset()));
                }

                if (m_file.has_multiple_object_versions() && m_file.type() != Osmium::OSMFile::FileType::Change()) {
                    check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "visible", object->visible() ? BAD_CAST "true" : BAD_CAST "false"));
                }
            }

            void write_tags(const Osmium::OSM::TagList& tags) {
                Osmium::OSM::TagList::const_iterator end = tags.end();
                for (Osmium::OSM::TagList::const_iterator it = tags.begin(); it != end; ++it) {
                    check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "tag")); // <tag>
                    check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "k", BAD_CAST it->key()));
                    check_for_error(xmlTextWriterWriteAttribute(m_xml_writer, BAD_CAST "v", BAD_CAST it->value()));
                    check_for_error(xmlTextWriterEndElement(m_xml_writer)); // </tag>
                }
            }

            void open_close_op_tag(const char op) {
                if (op == m_last_op) {
                    return;
                }

                if (m_last_op) {
                    check_for_error(xmlTextWriterEndElement(m_xml_writer));
                }

                switch (op) {
                    case 'c':
                        check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "create"));
                        break;
                    case 'm':
                        check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "modify"));
                        break;
                    case 'd':
                        check_for_error(xmlTextWriterStartElement(m_xml_writer, BAD_CAST "delete"));
                        break;
                }

                m_last_op = op;
            }

        }; // class XML

        namespace {

            inline Osmium::Output::Base* CreateOutputXML(const Osmium::OSMFile& file) {
                return new Osmium::Output::XML(file);
            }

            const bool xml_registered = Osmium::Output::Factory::instance().register_output_format(Osmium::OSMFile::FileEncoding::XML(),    CreateOutputXML) &&
                                        Osmium::Output::Factory::instance().register_output_format(Osmium::OSMFile::FileEncoding::XMLgz(),  CreateOutputXML) &&
                                        Osmium::Output::Factory::instance().register_output_format(Osmium::OSMFile::FileEncoding::XMLbz2(), CreateOutputXML);

        } // namespace

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_XML_HPP
