#include "PNG_RGBA.h"

PNG_RGBA::PNG_RGBA(const std::string &filePath) {
    // Open stream at file path.
    std::FILE *fp = fopen(filePath.c_str(), "rb");

    /* If file path does not point to a valid
     *   file or could not be opened, throw. */
    if (fp == nullptr)
        throw BadPath();

    // If the file is not a PNG, throw.
    if (!PNG_Loader::fileIsPNG(fp))
        throw NotPNG();

    // Setup LibPNG's PNG and INFO structs. If a problem is encountered, throw.
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                 (png_voidp) nullptr/*user_error_ptr*/,
                                                 nullptr/*user_error_fn*/,
                                                 nullptr/*user_warning_fn*/);
    if (!png_ptr)
        throw std::runtime_error("Internal Error: Could not create PNG object");
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp) nullptr, (png_infopp) nullptr);
        throw std::runtime_error("Internal Error: Could not create info object");
    }
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) nullptr);
        throw std::runtime_error("Internal Error: Could not create end info object");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        throw std::runtime_error("Exception: jumped");
    }

    // Load the image's properties.
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 0);
    png_read_info(png_ptr, info_ptr);
    selfInfo.width = png_get_image_width(png_ptr, info_ptr);
    selfInfo.height = png_get_image_height(png_ptr, info_ptr);
    selfInfo.colorDepth = png_get_bit_depth(png_ptr, info_ptr);
    selfInfo.numberOfPasses = (unsigned long int) png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    switch (png_get_color_type(png_ptr, info_ptr)) {
        case 0:
            selfInfo.colorType = PNG_ColorType::grayscale;
            break;
        case 2:
            selfInfo.colorType = PNG_ColorType::RGB_truecolor;
            break;
        case 3:
            selfInfo.colorType = PNG_ColorType::indexed;
            break;
        case 4:
            selfInfo.colorType = PNG_ColorType::grayscale_alpha;
            break;
        case 6:
            selfInfo.colorType = PNG_ColorType::RGBA;
            break;
        default:
            throw UnsupportedColorMode();
    }

    // Prepare a 2-D array for LibPNG to load the image data into.
    auto rowPointers = (png_bytep *) malloc(sizeof(png_bytep) * selfInfo.height);
    for (unsigned long int y = 0; y < selfInfo.height; y++)
        rowPointers[y] = (png_byte *) malloc(png_get_rowbytes(png_ptr, info_ptr));

    // Load image data.
    png_read_image(png_ptr, rowPointers);
    fclose(fp);

    // Load transfer data from 2-D array to a 1-D array.
    pngData = PNG_RGB_Array((unsigned long long int) selfInfo.height * (unsigned long long int) selfInfo.width);
    for (unsigned int y = 0; y < selfInfo.height; y++) {
        for (unsigned int x = 0; x < selfInfo.width; x++) {
            auto a = getRGBA_raw(x, y, rowPointers);
            pngData.at(getIndex(x, y, selfInfo.width)) = a;
        }
    }

    // Free 2-D array.
    for (unsigned long int y = 0; y < selfInfo.height; y++)
        free(rowPointers[y]);
    free(rowPointers);
}

PNG_RGBA::~PNG_RGBA() {
    //delete[] pngData;
}

PNG_Info PNG_RGBA::getInfo() {
    return selfInfo;
}

void PNG_RGBA::write_png_file(const std::string &file_path) {
    // Create stream at file path.
    FILE *fp = fopen(file_path.c_str(), "wb");

    /* If stream could not be create
     *   at file path, throw */
    if (!fp)
        throw BadPath();

    // Setup LibPNG's PNG and INFO structs. If a problem is encountered, throw.
    auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        throw std::runtime_error("Internal Error: Could not create PNG object");
    auto info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        throw std::runtime_error("Internal Error: Could not create info object");
    if (setjmp(png_jmpbuf(png_ptr)))
        throw std::runtime_error("Exception: jumped");
    png_init_io(png_ptr, fp);
    if (setjmp(png_jmpbuf(png_ptr)))
        throw std::runtime_error("Exception: jumped");

    // Set and load the output settings.
    png_set_IHDR(png_ptr, info_ptr, selfInfo.width, selfInfo.height,
                 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
        throw std::runtime_error("Exception: jumped");

    // Prepare a 2-D array for LibPNG to load the image data from.
    auto rows = new png_bytep[selfInfo.height];
    for (unsigned long int y = 0; y < selfInfo.height; y++) {
        rows[y] = (png_byte *) malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    // Transfer the image data into the LibPNG array.
    for (unsigned long int y = 0; y < selfInfo.height; y++) {
        for (unsigned long int x = 0; x < selfInfo.width; x++) {
            auto pixel = getPixel(x, y).value();
            setRGBA_raw(x, y, rows, pixel);
        }
    }

    // Write image to disk.
    png_write_image(png_ptr, rows);
    if (setjmp(png_jmpbuf(png_ptr)))
        throw std::runtime_error("Could not create image");
    png_write_end(png_ptr, nullptr);

    // Close output stream.
    fclose(fp);
}

std::optional<RGBA_Pixel> PNG_RGBA::getPixel(unsigned long int x, unsigned long int y) {
    // If x or y are outside the image bounds, return nothing.
    if ((x >= selfInfo.width) || (y >= selfInfo.height))
        return std::nullopt;

    // Return the pixel.
    return pngData.at(getIndex(x, y, selfInfo.width));
}

bool PNG_RGBA::setPixel(unsigned long x, unsigned long y, RGBA_Pixel &value) {
    // If x or y are outside the image bounds, return false.
    if ((x >= selfInfo.width) || (y >= selfInfo.height))
        return false;

    // Set the pixel to the supplied value.
    pngData.at(getIndex(x, y, selfInfo.width)) = value;
    return true;
}

RGBA_Pixel PNG_RGBA::getRGBA_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array) {
    // LibPNG magic.
    png_byte *row = PNG_array[y];
    png_byte *ptr = &(row[x * 4]);

    // Return the pixel data.
    return RGBA_Pixel{ptr[0], ptr[1], ptr[2], ptr[3]};
}

void PNG_RGBA::setRGBA_raw(unsigned long int x, unsigned long int y, png_bytepp PNG_array, RGBA_Pixel &pixel) {
    // LibPNG magic.
    png_byte *row = PNG_array[y];
    png_byte *ptr = &(row[x * 4]);

    // Set the pixel values to the ones supplied.
    ptr[0] = (png_byte) pixel.red;
    ptr[1] = (png_byte) pixel.green;
    ptr[2] = (png_byte) pixel.blue;
    ptr[3] = (png_byte) pixel.alpha;
}

unsigned long int PNG_RGBA::getIndex(unsigned long x, unsigned long y, unsigned long width) {
    return x + (y * width);
}

