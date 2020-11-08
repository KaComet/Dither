#ifndef DITHER_PNG_STRUCTS_H
#define DITHER_PNG_STRUCTS_H

#include <exception>

enum class PNG_ColorType {
    grayscale,
    RGB_truecolor,
    indexed,
    grayscale_alpha,
    RGBA,
};

struct PNG_Info {
    PNG_ColorType colorType;
    unsigned int colorDepth;
    unsigned long int width, height, numberOfPasses;

};

struct RGBA_Pixel {
    unsigned int red, green, blue, alpha;
};

struct RGB_Pixel {
    unsigned int red, green, blue;
};

typedef unsigned int GreyPixel;

struct HSV_Color {
    unsigned int hue, sat, value;
};

struct BadPath : public std::exception
{
    [[nodiscard]] const char * what () const noexcept override
    {
        return "Could not open file";
    }
};

struct NotPNG : public std::exception
{
    [[nodiscard]] const char * what () const noexcept override
    {
        return "File not PNG";
    }
};

struct UnsupportedColorMode : public std::exception
{
    [[nodiscard]] const char * what () const noexcept override
    {
        return "Color mode not supported";
    }
};


#endif //DITHER_PNG_STRUCTS_H
