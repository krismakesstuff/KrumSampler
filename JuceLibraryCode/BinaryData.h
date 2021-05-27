/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   MONOGLYCERIDE_TTF;
    const int            MONOGLYCERIDE_TTFSize = 45592;

    extern const char*   AIRBORNE_GP_ttf;
    const int            AIRBORNE_GP_ttfSize = 21160;

    extern const char*   MONOGLYCERIDEEXTRABOLD_TTF;
    const int            MONOGLYCERIDEEXTRABOLD_TTFSize = 45988;

    extern const char*   MONOGLYCERIDEBOLD_TTF;
    const int            MONOGLYCERIDEBOLD_TTFSize = 46308;

    extern const char*   MONOGLYCERIDEDEMIBOLD_TTF;
    const int            MONOGLYCERIDEDEMIBOLD_TTFSize = 46244;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
