#include "PNG_RGB_Array.h"

PNG_RGB_Array::PNG_RGB_Array(unsigned long long int nPixels) {
    _data = new RGBA_Pixel[nPixels];
    _nPixels = nPixels;
}

// Copy constructor
PNG_RGB_Array::PNG_RGB_Array(const PNG_RGB_Array &source) {
    _data = nullptr;
    _nPixels = 0;
    operator=(source);
}


PNG_RGB_Array::~PNG_RGB_Array() {
    delete[] _data;
}

RGBA_Pixel &PNG_RGB_Array::at(unsigned long long int n) {
    return _data[n];
}