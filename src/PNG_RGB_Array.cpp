#include "PNG_RGB_Array.h"

PNG_RGB_Array::PNG_RGB_Array(unsigned long long int nPixels, unsigned int nBits) : _nBits(nBits), _nPixels(nPixels) {
    _data = new RGBA_Pixel[nPixels];
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

RGBA_Pixel PNG_RGB_Array::atC(unsigned long long int n) const {
    return _data[n];
}

unsigned int PNG_RGB_Array::getDepthInBits() const noexcept {
    return _nBits;
}
