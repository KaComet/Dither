#ifndef DITHER_PNG_Grey_H
#define DITHER_PNG_Grey_H

#include <png.h>
#include <string>
#include <optional>
#include <stdexcept>
#include "PNG_Loader.h"
#include "PNG_structs.h"
#include "PNG_Data_Array.h"

class PNG_Grey {
public:
    PNG_Grey();

    PNG_Grey(unsigned long int width, unsigned long int height, unsigned int colorDepth);

    explicit PNG_Grey(const std::string &filePath);

    ~PNG_Grey() = default;

    /* Returns the grey value of the indicated pixel. Returns nothing if pixel is
     *   outside the bounds of the image. */
    [[nodiscard]] std::optional<GreyPixel> getPixel(unsigned long int x, unsigned long int y) const noexcept;

    /* Sets the pixel at x and y to the indicated grey value. Returns true if
     *   successful. Returns false if x or y are outside the bounds of the image. */
    bool setPixel(unsigned long int x, unsigned long int y, GreyPixel value);

    /* Returns the a struct containing
     *   the properties of the image. */
    [[nodiscard]] PNG_Info getInfo() const noexcept;

    // Writes the PNG file to the disk at the supplied file path.
    void write_png_file(const std::string &file_path) const;

private:
    /* Returns the grey pixel value at an x and y for a given LibPNG png_bytepp array. Used for
     *   converting the weird LibPNG format to a more efficient 1-D grey array. */
    static GreyPixel getGrey_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, unsigned int nBytesPerColor = 1);

    // sets a pixel at an x and y for a given LibPNG png_bytepp array. Used for writing images to disk
    static void setGrey_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, GreyPixel pixel,
                            unsigned int nBytesPerColor);

    static void setGreyRawTiny(unsigned long int x, unsigned long int y, png_bytepp PNG_array, GreyPixel pixel,
                                  unsigned int nColorsInByte);

    // Gets the index for a 1-D grey array for a given x and y.
    static unsigned long int getIndex(unsigned long int x, unsigned long int y, unsigned long width);

    PNG_Info selfInfo{};  // Image properties.
    PNG_Data_Array<GreyPixel> pngData = PNG_Data_Array<GreyPixel>(1, 0); // 1-D RGB array, the image's RGB values.
};


#endif //DITHER_PNG_Grey_H
