#include "PNG_Loader.h"
#include <stdexcept>
#include <cstdio>
#include <png.h>
#include "PNG_structs.h"

PNG_Info PNG_Loader::IdentifyPNG(const std::string &filePath) {
        // Setup LibPNG's PNG and INFO structs. If a problem is encountered, throw.
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                 (png_voidp) nullptr/*user_error_ptr*/,
                                                 nullptr/*user_error_fn*/,
                                                 nullptr/*user_warning_fn*/);
    if (!png_ptr) {
        throw std::runtime_error("Internal Error: Could not create PNG object");
    }
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
        throw std::runtime_error("Exception: jumped");
    }

    PNG_Info result{};

    // Open stream at file path.
    std::FILE *fp = fopen(filePath.c_str(), "rb");

    /* If file path does not point to a valid
     *   file or could not be opened, throw. */
    if (fp == nullptr) {
        throw BadPath();
    }

    // If the file is not a PNG, throw.
    if (!fileIsPNG(fp)) {
        fclose(fp);
        throw NotPNG();
    }

    // Load the image's properties.
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 0);
    png_read_info(png_ptr, info_ptr);
    result.width = png_get_image_width(png_ptr, info_ptr);
    result.height = png_get_image_height(png_ptr, info_ptr);
    result.colorDepth = png_get_bit_depth(png_ptr, info_ptr);
    result.numberOfPasses = (unsigned long int) png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    switch (png_get_color_type(png_ptr, info_ptr)) {
        case 0:
            result.colorType = PNG_ColorType::grayscale;
            break;
        case 2:
            result.colorType = PNG_ColorType::RGB_truecolor;
            break;
        case 3:
            result.colorType = PNG_ColorType::indexed;
            break;
        case 4:
            result.colorType = PNG_ColorType::grayscale_alpha;
            break;
        case 6:
            result.colorType = PNG_ColorType::RGBA;
            break;
        default:
            fclose(fp);
            throw UnsupportedColorMode();
    }
    fclose(fp);

    return result;
}

bool PNG_Loader::fileIsPNG(std::FILE *file_pointer) {
    char header[8];
    fpos_t position;

    // Return false if the pointer is NULL.
    if (file_pointer == nullptr)
        return false;

    fgetpos(file_pointer, &position); // Save the position of the stream.
    rewind(file_pointer); // Reset the stream to the beginning.
    fread(header, 1, 8, file_pointer); // Read the first 8 bytes of the stream.
    fsetpos(file_pointer, &position); // Reset the stream to the original position.

    // Return true if the stream is a PNG.
    return !png_sig_cmp((png_const_bytep) header, 0, 8);
}

