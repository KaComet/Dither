#include "ColorPalette.h"
#include "PNG_structs.h"
#include <cmath>

RGB_Pixel ColorPalette::getNearest(RGB_Pixel color) {
    if (colorpalette.empty())
        return RGB_Pixel{0, 0, 0};

    RGB_Pixel result = colorpalette.at(0);
    unsigned long int resultError = getError(color, result);

    for (const auto &thisColor : colorpalette) {
        unsigned long int thisError = getError(thisColor, color);
        if (thisError < resultError) {
            result = thisColor;
            resultError = thisError;
        }
    }

    return result;
}

RGBA_Pixel ColorPalette::getNearest(RGBA_Pixel color) {
    RGB_Pixel r = getNearest(RGB_Pixel{color.red, color.green, color.blue});
    return RGBA_Pixel{r.red, r.green, r.blue, color.alpha};
}


void ColorPalette::addColor(RGB_Pixel color) {
    colorpalette.push_back(color);
}

unsigned long int ColorPalette::getError(const RGB_Pixel &a, RGB_Pixel b) {
    b = vibrant(b);
    long int dR = (long int) a.red - (long int) b.red;
    long int dG = (long int) a.green - (long int) b.green;
    long int dB = (long int) a.blue - (long int) b.blue;
    auto d = (unsigned long int) sqrt((dR * dR) + (dG * dG) + (dB * dB));
    //auto d = dR + dG + dB;
    return d;
}

RGB_Pixel ColorPalette::vibrant(RGB_Pixel input) {
    double max;
    if ((input.red >= input.blue) && (input.red >= input.green))
        max = input.red;
    else if ((input.blue >= input.red) && (input.blue >= input.green))
        max = input.blue;
    else
        max = input.green;

    input.red = (unsigned int)round(0xFF * ((double)input.red / max));
    input.blue = (unsigned int)round(0xFF * ((double)input.blue / max));
    input.green = (unsigned int)round(0xFF * ((double)input.green / max));
    return input;
}

