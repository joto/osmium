#ifndef OSMIUM_OSM_TAG_HPP
#define OSMIUM_OSM_TAG_HPP

namespace Osmium {

    namespace OSM {

        /**
        *  An OSM tag. Has static allocation with enough memory to hold key and
        *  value strings.
        */
        class Tag {

          public:

            static const int max_characters_key   = 255;
            static const int max_characters_value = 255;

            static const int max_utf16_length_key   = 2 * (max_characters_key   + 1); ///< maximum number of UTF-16 units
            static const int max_utf16_length_value = 2 * (max_characters_value + 1);

            static const int max_length_key   = 4 * max_characters_key   + 1; ///< 255 UTF-8 characters + null byte
            static const int max_length_value = 4 * max_characters_value + 1; ///< 255 UTF-8 characters + null byte

            char key[max_length_key];     ///< Tag key
            char value[max_length_value]; ///< Tag value
          
        };

    } // namespace OSM

} // namespace Osmium

#endif // OSMIUM_OSM_TAG_HPP
