#include "PNG_RGB.h"
#include <cmath>

PNG_RGB::PNG_RGB() {
    pngData = PNG_Data_Array<RGB_Pixel>(25, 1);
    selfInfo.colorDepth = 0x8;
    selfInfo.colorType = PNG_ColorType::RGB_truecolor;
    selfInfo.width = 5;
    selfInfo.height = 5;
    selfInfo.numberOfPasses = 1;
}

PNG_RGB::PNG_RGB(const std::string &filePath) {
    // Setup LibPNG's PNG and INFO structs. If a problem is encountered, throw.
    std::pair<png_structp, png_infop> infoPair = std::pair<png_structp, png_infop>(nullptr, nullptr);

    try {
        infoPair = PNG_Loader::getLibPNGReadStructs();
    } catch (std::exception &ex) {
        throw ex;
    }
    png_structp png_ptr = infoPair.first;
    png_infop info_ptr = infoPair.second;

    // Open stream at file path.
    std::FILE *fp = fopen(filePath.c_str(), "rb");

    /* If file path does not point to a valid
     *   file or could not be opened, throw. */
    if (fp == nullptr) {
        throw BadPath();
    }

    // If the file is not a PNG, throw.
    if (!PNG_Loader::fileIsPNG(fp)) {
        fclose(fp);
        throw NotPNG();
    }

    transformToRGB(png_ptr, info_ptr, fp);

    // Load the image's final properties.
    PNG_Info finalInfo{};
    try {
        finalInfo = PNG_Loader::getPNGInfo(png_ptr, info_ptr);
    } catch (UnsupportedColorMode &e) {
        fclose(fp);
        throw e;
    }

    // Prepare a 2-D array for LibPNG and load the image data into it.
    png_bytepp rowPointers = PNG_Loader::loadPNGIntoRowPointers(finalInfo, png_ptr, info_ptr);
    fclose(fp);
    selfInfo = finalInfo;

    unsigned int nBytesPerPixel = PNG_Loader::getBytesPerPixel(selfInfo);

    // Load transfer data from 2-D array to a 1-D array.
    pngData = PNG_Data_Array<RGB_Pixel>(
            (unsigned long long int) selfInfo.height * (unsigned long long int) selfInfo.width,
            selfInfo.colorDepth);
    for (unsigned int y = 0; y < selfInfo.height; y++) {
        for (unsigned int x = 0; x < selfInfo.width; x++) {
            auto a = getRGB_raw(x, y, rowPointers, nBytesPerPixel);
            pngData.at(getIndex(x, y, selfInfo.width)) = a;
        }
    }

    // Free the memory used to load the PNG.
    PNG_Loader::FreeRowPointers(rowPointers, selfInfo);
}

PNG_Info PNG_RGB::getInfo() const noexcept {
    return selfInfo;
}

void PNG_RGB::write_png_file(const std::string &file_path) {
    // Setup LibPNG's PNG and INFO structs. If a problem is encountered, throw.
    std::pair<png_structp, png_infop> infoPair = std::pair<png_structp, png_infop>(nullptr, nullptr);
    try {
        infoPair = PNG_Loader::getLibPNGWriteStructs();
    } catch (std::exception &ex) {
        throw ex;
    }
    png_structp png_ptr = infoPair.first;
    png_infop info_ptr = infoPair.second;


    // Create stream at file path.
    FILE *fp = fopen(file_path.c_str(), "wb");

    /* If stream could not be created
     *   at file path, throw */
    if (fp == nullptr) {
        throw BadPath();
    }
    png_init_io(png_ptr, fp);
    if (setjmp(png_jmpbuf(png_ptr))) {
        fclose(fp);
        throw std::runtime_error("Exception: jumped");
    }

    // Set and load the output settings.
    png_set_IHDR(png_ptr, info_ptr, selfInfo.width, selfInfo.height,
                 pngData.getDepthInBits(), PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr))) {
        fclose(fp);
        throw std::runtime_error("Exception: jumped");
    }

    // Prepare a 2-D array for LibPNG to load the image data from.
    png_bytepp rowPointers = PNG_Loader::makeRowPointers(selfInfo, png_ptr, info_ptr);

    unsigned int nBytesPerPixel = PNG_Loader::getBytesPerPixel(selfInfo);

    // Transfer the image data into the LibPNG array.
    for (unsigned long int y = 0; y < selfInfo.height; y++) {
        for (unsigned long int x = 0; x < selfInfo.width; x++) {
            auto pixel = getPixel(x, y).value();
            setRGB_raw(x, y, rowPointers, pixel, nBytesPerPixel);
        }
    }

    // Write image to disk.
    png_write_image(png_ptr, rowPointers);
    if (setjmp(png_jmpbuf(png_ptr)))
        throw std::runtime_error("Could not create image");
    png_write_end(png_ptr, nullptr);

    // Close output stream.
    fclose(fp);
}

std::optional<RGB_Pixel> PNG_RGB::getPixel(unsigned long int x, unsigned long int y) const noexcept {
    // If x or y are outside the image bounds, return nothing.
    if ((x >= selfInfo.width) || (y >= selfInfo.height))
        return std::nullopt;

    // Return the pixel.
    return pngData.atC(getIndex(x, y, selfInfo.width));
}

bool PNG_RGB::setPixel(unsigned long x, unsigned long y, RGB_Pixel &value) {
    // If x or y are outside the image bounds, return false.
    if ((x >= selfInfo.width) || (y >= selfInfo.height))
        return false;

    // Set the pixel to the supplied value.
    pngData.at(getIndex(x, y, selfInfo.width)) = value;
    return true;
}

RGB_Pixel
PNG_RGB::getRGB_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, unsigned int nBytesPerColor) {
    // LibPNG magic.
    png_byte *row = PNG_array[y];
    png_byte *ptr = &(row[x * 3 * nBytesPerColor]);

    RGB_Pixel result = RGB_Pixel{0, 0, 0};
    for (unsigned int i = 0; i < nBytesPerColor; i++)
        result.red += ptr[(0 * nBytesPerColor) + i] << (((nBytesPerColor - 1) - i) * 8);

    for (unsigned int i = 0; i < nBytesPerColor; i++)
        result.green += ptr[(1 * nBytesPerColor) + i] << (((nBytesPerColor - 1) - i) * 8);

    for (unsigned int i = 0; i < nBytesPerColor; i++)
        result.blue += ptr[(2 * nBytesPerColor) + i] << (((nBytesPerColor - 1) - i) * 8);

    // Return the pixel data.
    return result;
}

void PNG_RGB::setRGB_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, RGB_Pixel &pixel,
                         unsigned int nBytesPerColor) {
    // LibPNG magic.
    png_byte *row = PNG_array[y];
    png_byte *ptr = &(row[x * 3 * nBytesPerColor]);

    for (unsigned int i = 0; i < nBytesPerColor; i++)
        ptr[(0 * nBytesPerColor) + i] = (pixel.red >> (((nBytesPerColor - 1) - i) * 8)) & (unsigned int) 0xFF;

    for (unsigned int i = 0; i < nBytesPerColor; i++)
        ptr[(1 * nBytesPerColor) + i] = (pixel.green >> (((nBytesPerColor - 1) - i) * 8)) & (unsigned int) 0xFF;

    for (unsigned int i = 0; i < nBytesPerColor; i++)
        ptr[(2 * nBytesPerColor) + i] = (pixel.blue >> (((nBytesPerColor - 1) - i) * 8)) & (unsigned int) 0xFF;
}

unsigned long int PNG_RGB::getIndex(unsigned long x, unsigned long y, unsigned long width) {
    return x + (y * width);
}

void PNG_RGB::transformToRGB(png_structp pngStructp, png_infop infoPtr, std::FILE *fp) {
    /* Load the image's properties. These will
     *   be used to identify any transformation
     *   that need to be applied to the image */
    png_init_io(pngStructp, fp);
    png_set_sig_bytes(pngStructp, 0);
    png_read_info(pngStructp, infoPtr);
    unsigned int colorDepth = png_get_bit_depth(pngStructp, infoPtr);

    // If the file is greyscale, convert to RGB.
    if (png_get_color_type(pngStructp, infoPtr) == PNG_COLOR_TYPE_GRAY) {
        png_set_gray_to_rgb(pngStructp);
    }

    // If the image has an alpha channel, remove it.
    if ((png_get_color_type(pngStructp, infoPtr) == PNG_COLOR_TYPE_RGBA) ||
        (png_get_color_type(pngStructp, infoPtr) == PNG_COLOR_TYPE_GRAY_ALPHA)) {
        png_set_strip_alpha(pngStructp);
    }

    // If the file is a palette image, convert to RGB
    if (png_get_color_type(pngStructp, infoPtr) == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(pngStructp);
        png_set_strip_alpha(pngStructp);
    }
}

