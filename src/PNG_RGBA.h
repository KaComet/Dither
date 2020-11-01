#ifndef DITHER_PNG_RGBA_H
#define DITHER_PNG_RGBA_H

#include <png.h>
#include <string>
#include <optional>
#include <stdexcept>
#include "PNG_Loader.h"
#include "PNG_structs.h"
#include "PNG_RGB_Array.h"

class PNG_RGBA {
public:
    PNG_RGBA();

    explicit PNG_RGBA(const std::string &filePath);

    ~PNG_RGBA() = default;

    /* Returns the RGB value of the indicated pixel. Returns nothing if pixel is
     *   outside the bounds of the image. */
    [[nodiscard]] std::optional<RGBA_Pixel> getPixel(unsigned long int x, unsigned long int y) const noexcept;

    /* Sets the pixel at x and y to the indicated RGB value. Returns true if
     *   successful. Returns false if x or y are outside the bounds of the image. */
    bool setPixel(unsigned long int x, unsigned long int y, RGBA_Pixel &value);

    /* Returns the a struct containing
     *   the properties of the image. */
    [[nodiscard]] PNG_Info getInfo() const noexcept;

    // Writes the PNG file to the disk at the supplied file path.
    void write_png_file(const std::string &file_path) const;

private:
    /* Returns the RGB pixel value at an x and y for a given LibPNG png_bytepp array. Used for
     *   converting the weird LibPNG format to a more efficient 1-D RGB array. */
    static RGBA_Pixel
    getRGBA_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, unsigned int nBytesPerColor = 1);

    // sets a pixel at an x and y for a given LibPNG png_bytepp array. Used for writing images to disk
    static void setRGBA_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, RGBA_Pixel &pixel,
                            unsigned int nBytesPerColor);

    // Gets the index for a 1-D RGB array for a given x and y.
    static unsigned long int getIndex(unsigned long int x, unsigned long int y, unsigned long width);

    PNG_Info selfInfo;  // Image properties.
    PNG_RGB_Array pngData = PNG_RGB_Array(1, 0); // 1-D RGB array, the image's RGB values.
};


#endif //DITHER_PNG_RGBA_H
