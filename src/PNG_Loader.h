#ifndef DITHER_PNG_LOADER_H
#define DITHER_PNG_LOADER_H

#include <string>
#include <cstdio>
#include <png.h>
#include "PNG_structs.h"

class PNG_Loader {
public:
    /* If the file is a PNG, returns the PNG's info. Throws if
     *   the file does not exist, or is not a PNG. */
    static PNG_Info IdentifyPNG(const std::string &filePath);

    // Returns true if the stream contains a PNG.
    static bool fileIsPNG(std::FILE *file_pointer);

    static PNG_Info getPNGInfo(png_structp pngStructp, png_infop infoPtr);

    static png_bytepp makeRowPointers(PNG_Info &info, png_structp pngStructp, png_infop infoPtr);

    static png_bytepp loadPNGIntoRowPointers(PNG_Info &info, png_structp pngStructp, png_infop infoPtr);

    static void FreeRowPointers(png_bytepp rowPointers, PNG_Info &info);

    static std::pair<png_structp, png_infop> getLibPNGReadStructs();

    static unsigned int getBytesPerPixel(PNG_Info &pngInfo) noexcept;

    static std::pair<png_structp, png_infop> getLibPNGWriteStructs();
};


#endif //DITHER_PNG_LOADER_H
