#ifndef OSMIUM_OUTPUT_SHAPEFILE_HPP
#define OSMIUM_OUTPUT_SHAPEFILE_HPP

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

#ifdef OSMIUM_WITH_SHPLIB

#include <shapefil.h>

namespace Osmium {

    namespace Output {

        class Shapefile {

        public:

            static const unsigned int MAX_DBF_FIELDS = 16;
            static const unsigned int MAX_FIELD_NAME_LENGTH = 11;
            static const int max_dbf_field_length = 255;

            int  num_fields;
            char field_name[MAX_DBF_FIELDS][MAX_FIELD_NAME_LENGTH+1];
            int  field_type[MAX_DBF_FIELDS];
            int  field_width[MAX_DBF_FIELDS];
            int  field_decimals[MAX_DBF_FIELDS];

            SHPHandle m_shp_handle;
            DBFHandle m_dbf_handle;

            v8::Persistent<v8::Object> js_object;

            Shapefile(const char *filename, int type) {
                num_fields = 0;
                m_shp_handle = SHPCreate(filename, type);
                assert(m_shp_handle != 0);
                m_dbf_handle = DBFCreate(filename);
                assert(m_dbf_handle != 0);

                std::string filen(filename);
                std::ofstream file;

                file.open(filen + ".prj");
                if (file.fail()) {
                    throw std::runtime_error("Can't open shapefile: " + filen);
                }
                file << "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]]" << std::endl;
                file.close();

                file.open(filen + ".cpg");
                if (file.fail()) {
                    throw std::runtime_error("Can't open shapefile: " + filen);
                }
                file << "UTF-8" << std::endl;
                file.close();

                js_object = v8::Persistent<v8::Object>::New( Osmium::Javascript::Template::create_output_shapefile_instance(this) );
                // js_object.MakeWeak((void *)(this), JS_Cleanup); // XXX doesn't work!?
            }

            virtual ~Shapefile() {
                close();
            }

            virtual SHPObject* add_geometry(Osmium::OSM::Object *object, std::string& transformation) = 0;

            /**
            * Define a field for a shapefile.
            */
            void add_field(const char *f_name, ///< The name of the field (1 to 11 characters long)
                           const char *f_type, ///< The type of the field ("string", "integer", "double", or "bool")
                           int f_width,        ///< The width of the field (number of digits for ints and doubles)
                           int f_decimals      ///< The precision of double fields (otherwise ignored)
                          ) {

                if (f_name[0] == '\0' || strlen(f_name) > MAX_FIELD_NAME_LENGTH) {
                    throw std::runtime_error("field name must be between 1 and 11 characters long");
                }

                DBFFieldType type;
                if (!strcmp(f_type, "string")) {
                    type = FTString;
                    f_decimals = 0;
                } else if (!strcmp(f_type, "integer")) {
                    type = FTInteger;
                    f_decimals = 0;
                } else if (!strcmp(f_type, "double")) {
                    type = FTDouble;
                } else if (!strcmp(f_type, "bool")) {
                    type = FTLogical;
                    f_width = 1;
                    f_decimals = 0;
                } else {
                    throw std::runtime_error("unknown field type");
                }

                int field_num = DBFAddField(m_dbf_handle, f_name, type, f_width, f_decimals);
                if (field_num < 0) {
                    throw std::runtime_error("failed to add field");
                }

                strcpy(field_name[field_num], f_name);
                field_type[field_num]     = type;
                field_width[field_num]    = f_width;
                field_decimals[field_num] = f_decimals;

                num_fields++;
            }

            int add_string_attribute(int ishape, int n, v8::Local<v8::Value> value) const {
                uint16_t source[(max_dbf_field_length+2)*2];
                char dest[(max_dbf_field_length+1)*4];
                memset(source, 0, (max_dbf_field_length+2)*4);
                memset(dest, 0, (max_dbf_field_length+1)*4);
                int32_t dest_length;
                UErrorCode error_code = U_ZERO_ERROR;
                value->ToString()->Write(source, 0, max_dbf_field_length+1);
                u_strToUTF8(dest, field_width[n], &dest_length, source, std::min(max_dbf_field_length+1, value->ToString()->Length()), &error_code);
                if (error_code == U_BUFFER_OVERFLOW_ERROR) {
                    // thats ok, it just means we clip the text at that point
                } else if (U_FAILURE(error_code)) {
                    throw std::runtime_error("UTF-16 to UTF-8 conversion failed");
                }
                return DBFWriteStringAttribute(m_dbf_handle, ishape, n, dest);
            }

            int add_logical_attribute(int ishape, int n, v8::Local<v8::Value> value) const {
                v8::String::Utf8Value str(value);

                if (atoi(*str) == 1 || !strncasecmp(*str, "T", 1) || !strncasecmp(*str, "Y", 1)) {
                    return DBFWriteLogicalAttribute(m_dbf_handle, ishape, n, 'T');
                } else if ((!strcmp(*str, "0")) || !strncasecmp(*str, "F", 1) || !strncasecmp(*str, "N", 1)) {
                    return DBFWriteLogicalAttribute(m_dbf_handle, ishape, n, 'F');
                } else {
                    return DBFWriteNULLAttribute(m_dbf_handle, ishape, n);
                }
            }

            /**
            * Add an %OSM object to the shapefile.
            */
            void add(Osmium::OSM::Object *object,      ///< the %OSM object (Node, Way, or Relation)
                     v8::Local<v8::Object> attributes, ///< a %Javascript object (hash) with the attributes
                     std::string& transformation
                    ) {

                SHPObject *shp_object = 0;
                int ishape;

                try {
                    shp_object = add_geometry(object, transformation);
                } catch (std::exception& e) { // XXX ignore errors when creating geometry
                    std::cerr << "ignoring error: " << e.what() << "\n";
                    return;
                }
                if (!shp_object) return; // XXX return if the shape is invalid
                ishape = SHPWriteObject(m_shp_handle, -1, shp_object);
                SHPDestroyObject(shp_object);

                int ok = 0;
                for (int n=0; n < num_fields; n++) {
                    v8::Local<v8::String> key = v8::String::New(field_name[n]);
                    if (attributes->HasRealNamedProperty(key)) {
                        v8::Local<v8::Value> value = attributes->GetRealNamedProperty(key);
                        if (value->IsUndefined() || value->IsNull()) {
                            DBFWriteNULLAttribute(m_dbf_handle, ishape, n);
                        } else {
                            switch (field_type[n]) {
                                case FTString:
                                    ok = add_string_attribute(ishape, n, value);
                                    break;
                                case FTInteger:
                                    ok = DBFWriteIntegerAttribute(m_dbf_handle, ishape, n, value->Int32Value());
                                    break;
                                case FTDouble:
                                    throw std::runtime_error("fields of type double not implemented");
                                    break;
                                case FTLogical:
                                    ok = add_logical_attribute(ishape, n, value);
                                    break;
                                default:
                                    ok = 0; // should never be here
                                    break;
                            }
                            if (!ok) {
                                std::string errmsg("failed to add attribute '");
                                errmsg += field_name[n];
                                errmsg += "'\n";
                                throw std::runtime_error(errmsg);
                            }
                        }
                    } else {
                        DBFWriteNULLAttribute(m_dbf_handle, ishape, n);
                    }
                }
            }

            void close() {
                if (m_dbf_handle) {
                    DBFClose(m_dbf_handle);
                    m_dbf_handle = 0;
                }
                if (m_shp_handle) {
                    SHPClose(m_shp_handle);
                    m_shp_handle = 0;
                }
            }

            v8::Handle<v8::Object> get_js_object() {
                return js_object;
            }

            v8::Handle<v8::Value> js_add_field(const v8::Arguments& args) {
                if (args.Length() < 3 || args.Length() > 4) {
                    throw std::runtime_error("wrong number of arguments");
                }
                v8::String::Utf8Value name(args[0]);
                v8::String::Utf8Value type(args[1]);
                int width = args[2]->Int32Value();
                int decimals = (args.Length() == 4) ? args[3]->Int32Value() : 0;
                add_field(*name, *type, width, decimals);

                return v8::Integer::New(1);
            }

            v8::Handle<v8::Value> js_add(const v8::Arguments& args) {
                if (args.Length() < 2 || args.Length() > 3) {
                    throw std::runtime_error("wrong number of arguments");
                }

                v8::Local<v8::Object> xxx = v8::Local<v8::Object>::Cast(args[0]);
                Osmium::OSM::Object *object = (Osmium::OSM::Object *) v8::Local<v8::External>::Cast(xxx->GetInternalField(0))->Value();

                std::string transformation;
                if (args.Length() == 3) {
                    v8::String::AsciiValue gt(args[2]);
                    transformation = *gt;
                }
                add(object, v8::Local<v8::Object>::Cast(args[1]), transformation);

                return v8::Integer::New(1);
            }

            v8::Handle<v8::Value> js_close(const v8::Arguments& /*args*/) {
                close();
                return v8::Undefined();
            }

        }; // class Shapefile

        class PointShapefile : public Shapefile {

        public:

            PointShapefile(const char* filename) : Shapefile(filename, SHPT_POINT) {
            }

            SHPObject* add_geometry(Osmium::OSM::Object *object, std::string& transformation) {
                return object->create_shp_point(transformation);
            }

        };

        class LineShapefile : public Shapefile {

        public:

            LineShapefile(const char* filename) : Shapefile(filename, SHPT_ARC) {
            }

            SHPObject* add_geometry(Osmium::OSM::Object *object, std::string& transformation) {
                return object->create_shp_line(transformation);
            }

        };

        class PolygonShapefile : public Shapefile {

        public:

            PolygonShapefile(const char* filename) : Shapefile(filename, SHPT_POLYGON) {
            }

            SHPObject* add_geometry(Osmium::OSM::Object *object, std::string& transformation) {
                return object->create_shp_polygon(transformation);
            }

        };

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_WITH_SHPLIB

#endif // OSMIUM_OUTPUT_SHAPEFILE_HPP
