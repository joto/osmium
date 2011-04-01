#ifndef OSMIUM_OUTPUT_SHAPEFILE_HPP
#define OSMIUM_OUTPUT_SHAPEFILE_HPP

#ifdef WITH_SHPLIB

#include <shapefil.h>

namespace Osmium {

    namespace Output {

        class Shapefile {

            public:

            static const unsigned int MAX_DBF_FIELDS = 16;
            static const unsigned int MAX_FIELD_NAME_LENGTH = 11;
            static const int max_dbf_field_length = 255;

            int  shp_type;
            int  num_fields;
            char field_name[MAX_DBF_FIELDS][MAX_FIELD_NAME_LENGTH+1];
            int  field_type[MAX_DBF_FIELDS];
            int  field_width[MAX_DBF_FIELDS];
            int  field_decimals[MAX_DBF_FIELDS];

            SHPHandle shp_handle;
            DBFHandle dbf_handle;

            v8::Persistent<v8::Object> js_object;

            public:

            static void JS_Cleanup(v8::Persistent<v8::Value> /*obj*/, void *self) { // XXX is never called
                printf("cleanup\n\n");
                ((Shapefile *)self)->close();
            }

            Shapefile(const char *filename, const char *shape_type) {
                if (!strcmp(shape_type, "point")) {
                    shp_type = SHPT_POINT;
                } else if (!strcmp(shape_type, "line")) {
                    shp_type = SHPT_ARC;
                } else if (!strcmp(shape_type, "polygon")) {
                    shp_type = SHPT_POLYGON;
                } else {
                    throw std::runtime_error("unkown shapefile type");
                }

                num_fields = 0;

                shp_handle = SHPCreate(filename, shp_type);
                assert(shp_handle != 0);
                dbf_handle = DBFCreate(filename);
                assert(dbf_handle != 0);

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

            ~Shapefile() {
                close();
            }

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

                int field_num = DBFAddField(dbf_handle, f_name, type, f_width, f_decimals);
                if (field_num < 0) {
                    throw std::runtime_error("failed to add field");
                }

                strcpy(field_name[field_num], f_name);
                field_type[field_num]     = type;
                field_width[field_num]    = f_width;
                field_decimals[field_num] = f_decimals;

                num_fields++;
            }

            /**
            * Add an %OSM object to the shapefile.
            */
            void add(Osmium::OSM::Object *object,     ///< the %OSM object (Node, Way, or Relation)
                     v8::Local<v8::Object> attributes ///< a %Javascript object (hash) with the attributes
                     ) {

                SHPObject *shp_object = 0;
                int ishape;

                try {
                    shp_object = object->create_shpobject(shp_type);
                } catch(std::exception& e) { // XXX ignore errors when creating geometry
                    std::cerr << "ignoring error: " << e.what() << "\n";
                    return;
                }
                if (!shp_object) return; // XXX return if the shape is invalid
                ishape = SHPWriteObject(shp_handle, -1, shp_object);
                SHPDestroyObject(shp_object);

                int ok = 0;
                for (int n=0; n < num_fields; n++) {
                    v8::Local<v8::String> key = v8::String::New(field_name[n]); 
                    if (attributes->HasRealNamedProperty(key)) {
                        v8::Local<v8::Value> value = attributes->GetRealNamedProperty(key);
                        if (value->IsUndefined() || value->IsNull()) {
                            DBFWriteNULLAttribute(dbf_handle, ishape, n);
                        } else {
                            switch (field_type[n]) {
                                case FTString: {
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
                                        ok = DBFWriteStringAttribute(dbf_handle, ishape, n, dest);
                                    }
                                    break;
                                case FTInteger:
                                    ok = DBFWriteIntegerAttribute(dbf_handle, ishape, n, value->Int32Value());
                                    break;
                                case FTDouble:
                                    throw std::runtime_error("fields of type double not implemented");
                                    break;
                                case FTLogical: {
                                        v8::String::Utf8Value str(value);

                                        if (atoi(*str) == 1 || !strncasecmp(*str, "T", 1) || !strncasecmp(*str, "Y", 1)) {
                                            ok = DBFWriteLogicalAttribute(dbf_handle, ishape, n, 'T');
                                        } else if ((!strcmp(*str, "0")) || !strncasecmp(*str, "F", 1) || !strncasecmp(*str, "N", 1)) {
                                            ok = DBFWriteLogicalAttribute(dbf_handle, ishape, n, 'F');
                                        } else {
                                            ok = DBFWriteNULLAttribute(dbf_handle, ishape, n);
                                        }
                                    }
                                    break;
                            }
                            // XXX whats this?
    /*                        if (!ok) {
                                std::string errmsg("failed to add attribute '");
                                errmsg += field_name[n];
                                errmsg += "'\n";
                                throw std::runtime_error(errmsg);
                            }*/
                        }
                    } else {
                        DBFWriteNULLAttribute(dbf_handle, ishape, n);
                    }
                }
            }

            void close() {
                DBFClose(dbf_handle);
                SHPClose(shp_handle);
            }

            v8::Handle<v8::Object> get_js_object() {
                return js_object;
            }

        }; // class Shapefile

    } // namespace Output

} // namespace Osmium

#endif // WITH_SHPLIB

#endif // OSMIUM_OUTPUT_SHAPEFILE_HPP
