#ifndef DITHER_PNG_RGBA_H
#define DITHER_PNG_RGBA_H

#include <png.h>
#include <string>
#include <optional>
#include "PNG_Loader.h"
#include "PNG_structs.h"
#include "PNG_RGB_Array.h"

class PNG_RGBA {
public:
    explicit PNG_RGBA(const std::string &filePath);

    ~PNG_RGBA();

    /* Returns the RGB value of the indicated pixel. Returns nothing if pixel is
     *   outside the bounds of the image. */
    std::optional<RGBA_Pixel> getPixel(unsigned long int x, unsigned long int y);

    /* Sets the pixel at x and y to the indicated RGB value. Returns true if
     *   successful. Returns false if x or y are outside the bounds of the image. */
    bool setPixel(unsigned long int x, unsigned long int y, RGBA_Pixel &value);

    /* Returns the a struct containing
     *   the properties of the image. */
    PNG_Info getInfo();

    // Writes the PNG file to the disk at the supplied file path.
    void write_png_file(const std::string &file_path);

private:
    /* Returns the RGB pixel value at an x and y for a given LibPNG png_bytepp array. Used for
     *   converting the weird LibPNG format to a more efficient 1-D RGB array. */
    static RGBA_Pixel getRGBA_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array);

    // sets a pixel at an x and y for a given LibPNG png_bytepp array. Used for writing images to disk
    static void setRGBA_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, RGBA_Pixel &pixel);

    // Gets the index for a 1-D RGB array for a given x and y.
    static unsigned long int getIndex(unsigned long int x, unsigned long int y, unsigned long width);

    PNG_Info selfInfo;  // Image properties.
    PNG_RGB_Array pngData = PNG_RGB_Array(1); // 1-D RGB array, the image's RGB values.
};


#endif //DITHER_PNG_RGBA_H