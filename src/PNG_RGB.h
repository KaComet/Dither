#ifndef DITHER_PNG_RGB_H
#define DITHER_PNG_RGB_H

#include <png.h>
#include <string>
#include <optional>
#include <stdexcept>
#include "PNG_Loader.h"
#include "PNG_structs.h"
#include "PNG_Data_Array.h"

class PNG_RGB {
public:
    PNG_RGB();

    explicit PNG_RGB(const std::string &filePath);

    ~PNG_RGB() = default;

    /* Returns the RGB value of the indicated pixel. Returns nothing if pixel is
     *   outside the bounds of the image. */
    [[nodiscard]] std::optional<RGB_Pixel> getPixel(unsigned long int x, unsigned long int y) const noexcept;

    /* Sets the pixel at x and y to the indicated RGB value. Returns true if
     *   successful. Returns false if x or y are outside the bounds of the image. */
    bool setPixel(unsigned long int x, unsigned long int y, RGB_Pixel &value);

    /* Returns the a struct containing
     *   the properties of the image. */
    [[nodiscard]] PNG_Info getInfo() const noexcept;

    // Writes the PNG file to the disk at the supplied file path.
    void write_png_file(const std::string &file_path);

private:
    /* Returns the RGB pixel value at an x and y for a given LibPNG png_bytepp array. Used for
     *   converting the weird LibPNG format to a more efficient 1-D RGB array. */
    static RGB_Pixel
    getRGB_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, unsigned int nBytesPerColor = 1);

    // sets a pixel at an x and y for a given LibPNG png_bytepp array. Used for writing images to disk
    static void setRGB_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, RGB_Pixel &pixel,
                           unsigned int nBytesPerColor);

    // Gets the index for a 1-D RGB array for a given x and y.
    static unsigned long int getIndex(unsigned long int x, unsigned long int y, unsigned long width);

    static void transformToRGB(png_structp pngStructp, png_infop infoPtr, std::FILE *fp);

    PNG_Info selfInfo{};  // Image properties.
    PNG_Data_Array<RGB_Pixel> pngData = PNG_Data_Array<RGB_Pixel>(1, 0); // 1-D RGB array, the image's RGB values.
};


#endif //DITHER_PNG_RGB_H
