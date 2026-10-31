/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   ProspektSP_png;
    const int            ProspektSP_pngSize = 1825633;

    extern const char*   ProspektSP_2_jpg;
    const int            ProspektSP_2_jpgSize = 325756;

    extern const char*   _Prospekt3_ico;
    const int            _Prospekt3_icoSize = 551466;

    extern const char*   AppIcon_png;
    const int            AppIcon_pngSize = 1025560;

    extern const char*   icon16_png;
    const int            icon16_pngSize = 836;

    extern const char*   icon24_png;
    const int            icon24_pngSize = 1546;

    extern const char*   icon32_png;
    const int            icon32_pngSize = 1914;

    extern const char*   icon48_png;
    const int            icon48_pngSize = 3246;

    extern const char*   icon64_png;
    const int            icon64_pngSize = 4571;

    extern const char*   icon128_png;
    const int            icon128_pngSize = 11860;

    extern const char*   icon256_png;
    const int            icon256_pngSize = 64243;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 11;

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
