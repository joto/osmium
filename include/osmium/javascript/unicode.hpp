#ifndef OSMIUM_JAVASCRIPT_UNICODE_HPP
#define OSMIUM_JAVASCRIPT_UNICODE_HPP

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

#include <cstdlib>
#include <ostream>

#include <v8.h>
#include <unicode/ustring.h>

namespace Osmium {

    /* These unicode conversion functions are used to convert UTF-8 to UTF-16 and then into
    a v8::String and back, because the functions that v8 has for this do not handle
    characters outside the Basic Multilingual Plane (>16bit) properly. */

    /// Parent exception class for Unicode conversion errors.
    class Unicode_Conversion_Error {

    public:

        UErrorCode error_code;
        Unicode_Conversion_Error(UErrorCode ec) :
            error_code(ec) { }

        /// Is this a buffer overflow?
        bool buffer_overflow() const {
            return error_code == U_BUFFER_OVERFLOW_ERROR;
        }

    }; // class Unicode_Conversion_Error

    /// Exception thrown when a UTF-8 to UTF-16 conversion failed.
    class UTF8_to_UTF16_Conversion_Error : public Unicode_Conversion_Error {

    public:

        UTF8_to_UTF16_Conversion_Error(UErrorCode ec) :
            Unicode_Conversion_Error(ec) { }

    }; // class UTF8_to_UTF16_Conversion_Error

    /// Exception thrown when a UTF-16 to UTF-8 conversion failed.
    class UTF16_to_UTF8_Conversion_Error : public Unicode_Conversion_Error {

    public:

        UTF16_to_UTF8_Conversion_Error(UErrorCode ec) :
            Unicode_Conversion_Error(ec) { }

    }; // class UTF16_to_UTF8_Conversion_Error

    /**
    * Convert C string with UTF-8 codes into v8::String.
    *
    * @exception UTF8_to_UTF16_Conversion_Error Thrown if the conversion failed.
    * @tparam characters Maximum number of Unicode characters.
    * @param cstring A NULL terminated C string.
    * @return A local handle to a v8 String.
    */
    template <int characters>
    inline v8::Local<v8::String> utf8_to_v8_String(const char* cstring) {
        UErrorCode error_code = U_ZERO_ERROR;
        UChar dest[characters*2];
        int32_t dest_length;
        u_strFromUTF8(dest, characters*2, &dest_length, cstring, -1, &error_code);
        if (error_code != U_ZERO_ERROR) {
            throw UTF8_to_UTF16_Conversion_Error(error_code);
        }
        return v8::String::New(dest, dest_length);
    }

    /**
    * Convert v8::String into C string with UTF-8 codes.
    *
    * @exception UTF16_to_UTF8_Conversion_Error Thrown if the conversion failed.
    * @tparam characters Maximum number of Unicode characters.
    * @param string A v8::String.
    * @return Returns a pointer to a static buffer with a NULL terminated C string.
    */
    template <int characters>
    inline const char* v8_String_to_utf8(v8::Local<v8::String> string) {
        UErrorCode error_code = U_ZERO_ERROR;
        uint16_t src[characters*2];
        static char buffer[characters*4];
        int32_t buffer_length;
        string->Write(src, 0, characters*2);
        u_strToUTF8(buffer, characters*4, &buffer_length, src, std::min(characters*2, string->Length()), &error_code);
        if (error_code != U_ZERO_ERROR) {
            throw UTF16_to_UTF8_Conversion_Error(error_code);
        }
        return buffer;
    }

    /**
    * Sends v8::String to output stream. This will first convert it to a UTF-8 string.
    *
    * @exception UTF16_to_UTF8_Conversion_Error Thrown if the conversion failed.
    * @param string A v8::String.
    * @param os A reference to an output stream.
    */
    inline void v8_String_to_ostream(v8::Local<v8::String> string, std::ostream& os) {
        UErrorCode error_code = U_ZERO_ERROR;
        int length = 4 * (string->Length() + 1);
        uint16_t* src = static_cast<uint16_t*>(malloc(length));
        if (!src) {
            throw std::bad_alloc();
        }
        char* buffer = static_cast<char*>(malloc(length));
        if (!buffer) {
            free(src);
            throw std::bad_alloc();
        }
        int32_t buffer_length;
        string->Write(src);
        u_strToUTF8(buffer, length, &buffer_length, src, string->Length(), &error_code);
        if (error_code != U_ZERO_ERROR) {
            free(buffer);
            free(src);
            throw UTF16_to_UTF8_Conversion_Error(error_code);
        }
        os << buffer;
        free(buffer);
        free(src);
    }

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_UNICODE_HPP
