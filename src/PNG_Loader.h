#ifndef DITHER_PNG_LOADER_H
#define DITHER_PNG_LOADER_H

#include <string>
#include <cstdio>
#include "PNG_structs.h"

class PNG_Loader {
public:
    /* If the file is a PNG, returns the PNG's info. Throws if
     *   the file does not exist, or is not a PNG. */
    static PNG_Info IdentifyPNG(const std::string &filePath);

    // Returns true if the stream contains a PNG.
    static bool fileIsPNG(std::FILE *file_pointer);
};


#endif //DITHER_PNG_LOADER_H
