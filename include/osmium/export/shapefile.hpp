#ifndef OSMIUM_EXPORT_SHAPEFILE_HPP
#define OSMIUM_EXPORT_SHAPEFILE_HPP

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

#include <fstream>
#include <sstream>
#include <cerrno>
#include <shapefil.h>
#include <boost/utility.hpp>

#include <osmium/geometry/shplib.hpp>

namespace Osmium {

    namespace Export {

        class Shapefile : boost::noncopyable {

        public:

            // the following limits are defined by the shapefile spec
            static const unsigned int max_dbf_field_name_length =  11;
            static const          int max_dbf_field_length      = 255;

            // this limit has been arrived at experimentally
            static const unsigned int max_dbf_fields = 2047;

        private:

            class Field {

            public:

                Field(const std::string& name, DBFFieldType type, int width=1, int decimals=0) : m_name(name), m_type(type), m_width(width), m_decimals(decimals) {
                    if (name == "" || name.size() > max_dbf_field_name_length) {
                        throw std::invalid_argument("field name must be between 1 and 11 characters long");
                    } else if (width > max_dbf_field_length) {
                        throw std::invalid_argument("field width must not exceed 255 characters");
                    }
                }

                const std::string& name() const {
                    return m_name;
                }

                DBFFieldType type() const {
                    return m_type;
                }

                int width() const {
                    return m_width;
                }

                int decimals() const {
                    return m_decimals;
                }

            private:

                std::string m_name;
                DBFFieldType m_type;
                int m_width;
                int m_decimals;

            };

        public:

            virtual ~Shapefile() {
                close();
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

            /**
            * Add a field to a shapefile.
            */
            void add_field(Field& field) {
                if (m_fields.size() < max_dbf_fields) {
                    int field_num = DBFAddField(m_dbf_handle, field.name().c_str(), field.type(), field.width(), field.decimals());
                    if (field_num != static_cast<int>(m_fields.size())) {
                        throw std::runtime_error("Failed to add field:" + field.name());
                    }
                    m_fields.push_back(field);
                } else {
                    throw std::out_of_range("Too many fields in the shapefile.");
                }
            }

            /**
            * Add a field to a shapefile.
            */
            void add_field(const std::string& name, ///< The name of the field (1 to 11 characters long)
                           DBFFieldType type,       ///< The type of the field (FT_STRING, FT_INTEGER, FT_DOUBLE, or FT_BOOL)
                           int width=1,             ///< The width of the field (number of digits for ints and doubles)
                           int decimals=0           ///< The precision of double fields (otherwise ignored)
                          ) {
                Field field(name, type, width, decimals);
                add_field(field);
            }

            /**
            * Add a field to a shapefile.
            */
            void add_field(const std::string& name, ///< The name of the field (1 to 11 characters long)
                           const std::string& type, ///< The type of the field ("string", "integer", "double", or "bool")
                           int width=1,             ///< The width of the field (number of digits for ints and doubles)
                           int decimals=0           ///< The precision of double fields (otherwise ignored)
                          ) {

                DBFFieldType ftype;
                if (type == "string") {
                    ftype = FTString;
                    decimals = 0;
                } else if (type == "integer") {
                    ftype = FTInteger;
                    decimals = 0;
                } else if (type == "double") {
                    ftype = FTDouble;
                } else if (type == "bool") {
                    ftype = FTLogical;
                    width = 1;
                    decimals = 0;
                } else {
                    throw std::runtime_error("Unknown field type:" + type);
                }

                add_field(name, ftype, width, decimals);
            }

            const std::vector<Field>& fields() const {
                return m_fields;
            }

            const Field& field(int n) const {
                return m_fields[n];
            }

            /**
             * Add a new geometry (shape object) to the Shapefile. You have to call
             * this first for every new shape. After that you call add_attribute()
             * for all the attributes.
             *
             * @param shp_object A pointer to the shape object to be added. The object
             *                   will be freed for you by calling SHPDestroyObject()!
             * @exception Osmium::Geometry::IllegalGeometry If shp_object is NULL or
             *                   the type of geometry does not fit the type of the
             *                   shapefile.
             */
            void add_geometry(SHPObject* shp_object) {
                if (!shp_object || shp_object->nSHPType != m_shp_handle->nShapeType) {
                    throw Osmium::Geometry::IllegalGeometry();
                }
                m_current_shape = SHPWriteObject(m_shp_handle, -1, shp_object);
                if (m_current_shape == -1 && errno == EINVAL) {
                    // second chance if likely cause is having reached the 2GB limit
                    close();
                    m_sequence_number++;
                    open();
                    m_current_shape = SHPWriteObject(m_shp_handle, -1, shp_object);
                }
                if (m_current_shape == -1) {
                    throw std::runtime_error("error writing to shapefile");
                }
                SHPDestroyObject(shp_object);
            }

            void add_attribute(const int field, const bool value) const {
                int ok = DBFWriteLogicalAttribute(m_dbf_handle, m_current_shape, field, value ? 'T' : 'F');
                if (!ok) {
                    throw std::runtime_error(std::string("Can't add bool to field"));
                }
            }

            void add_attribute(const int field, const int value) const {
                int ok = DBFWriteIntegerAttribute(m_dbf_handle, m_current_shape, field, value);
                if (!ok) {
                    throw std::runtime_error(std::string("Can't add integer to field"));
                }
            }

            void add_attribute(const int field, const std::string& value) const {
                int ok = DBFWriteStringAttribute(m_dbf_handle, m_current_shape, field, value.c_str());
                if (!ok) {
                    throw std::runtime_error(std::string("Can't add string to field"));
                }
            }

            void add_attribute(const int field, const char *value) const {
                int ok = DBFWriteStringAttribute(m_dbf_handle, m_current_shape, field, value);
                if (!ok) {
                    throw std::runtime_error(std::string("Can't add char* to field"));
                }
            }

            void add_attribute(const int field) const {
                int ok = DBFWriteNULLAttribute(m_dbf_handle, m_current_shape, field);
                if (!ok) {
                    throw std::runtime_error(std::string("Can't add null to field"));
                }
            }

            // truncates UTF8 string to fit in shape field
            void add_attribute_with_truncate(const int field, const char* value) {
                char dest[max_dbf_field_length+1];
                size_t length = m_fields[field].width();
                memset(dest, 0, length+1);
                strncpy(dest, value, length);
                size_t i = length-1;
                if (dest[i] & 128) {
                    if (dest[i] & 64) {
                        dest[i] = '\0';
                    } else if ((dest[i-1] & 224) == 224) {
                        dest[i-1] = '\0';
                    } else if ((dest[i-2] & 240) == 240) {
                        dest[i-2] = '\0';
                    }
                }
                add_attribute(field, dest);
            }

            void add_attribute_with_truncate(const int field, const std::string& value) {
                add_attribute_with_truncate(field, value.c_str());
            }

        protected:

            /**
             * The constructor for Shapefile is protected. Use one of
             * PointShapefile, LineShapefile, or PolygonShapefile.
             */
            Shapefile(const std::string& filename, int type) :
                m_filename_base(filename),
                m_fields(),
                m_shp_handle(NULL),
                m_dbf_handle(NULL),
                m_current_shape(0),
                m_type(type),
                m_sequence_number(0) {
                open();
            }

        private:

            /// base filename
            const std::string m_filename_base;

            /// fields in DBF
            std::vector<Field> m_fields;

            // handles to the shapelib objects
            SHPHandle m_shp_handle;
            DBFHandle m_dbf_handle;

            /// entity number of the shape we are currently writing
            int m_current_shape;

            /// shapefile type
            int m_type;

            /// shapefile sequence number for auto-overflow (0=first)
            int m_sequence_number;

            /**
             * Open and initialize all files belonging to shapefile (.shp/shx/dbf/prj/cpg).
             * Uses m_filename_base and m_sequence_number plus suffix to build filename.
             */
            void open() {
                std::ostringstream filename;
                filename << m_filename_base;
                if (m_sequence_number) {
                    filename << "_" << m_sequence_number;
                }

                m_shp_handle = SHPCreate(filename.str().c_str(), m_type);
                if (m_shp_handle == 0) {
                    throw std::runtime_error("Can't open shapefile: " + filename.str() + ".shp/shx");
                }
                m_dbf_handle = DBFCreate(filename.str().c_str());
                if (m_dbf_handle == 0) {
                    throw std::runtime_error("Can't open shapefile: " + filename.str() + ".dbf");
                }

                std::ofstream file;
                file.open((filename.str() + ".prj").c_str());
                if (file.fail()) {
                    throw std::runtime_error("Can't open shapefile: " + filename.str() + ".prj");
                }
                file << "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]]" << std::endl;
                file.close();

                file.open((filename.str() + ".cpg").c_str());
                if (file.fail()) {
                    throw std::runtime_error("Can't open shapefile: " + filename.str() + ".cpg");
                }
                file << "UTF-8" << std::endl;
                file.close();

                // If any fields are defined already, add them here. This will do nothing if
                // called from the constructor.
                for (std::vector<Field>::const_iterator it = m_fields.begin(); it != m_fields.end(); ++it) {
                    DBFAddField(m_dbf_handle, it->name().c_str(), it->type(), it->width(), it->decimals());
                }
            }

        }; // class Shapefile

        /**
         * Shapefile containing point geometries.
         */
        class PointShapefile : public Shapefile {

        public:

            /**
             * Create shapefile.
             *
             * @param filename Filename (optionally including path) without any suffix.
             */
            PointShapefile(const std::string& filename) : Shapefile(filename, SHPT_POINT) {
            }

        };

        /**
         * Shapefile containing line geometries.
         */
        class LineStringShapefile : public Shapefile {

        public:

            /**
             * Create shapefile.
             *
             * @param filename Filename (optionally including path) without any suffix.
             */
            LineStringShapefile(const std::string& filename) : Shapefile(filename, SHPT_ARC) {
            }

        };

        /**
         * Shapefile containing polygon geometries.
         */
        class PolygonShapefile : public Shapefile {

        public:

            /**
             * Create shapefile.
             *
             * @param filename Filename (optionally including path) without any suffix.
             */
            PolygonShapefile(const std::string& filename) : Shapefile(filename, SHPT_POLYGON) {
            }

        };

    } // namespace Export

} // namespace Osmium

#endif // OSMIUM_EXPORT_SHAPEFILE_HPP
