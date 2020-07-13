#ifndef DITHER_COLORPALETTE_H
#define DITHER_COLORPALETTE_H

#include <vector>
#include "PNG_structs.h"

class ColorPalette {
public:
    RGB_Pixel getNearest(RGB_Pixel color);
    RGBA_Pixel getNearest(RGBA_Pixel color);
    void addColor(RGB_Pixel color);

private:
    std::vector<RGB_Pixel> colorpalette;
    static unsigned long int getError(const RGB_Pixel &a, RGB_Pixel b);
    static RGB_Pixel vibrant(RGB_Pixel);
};


#endif //DITHER_COLORPALETTE_H
