#include "PNG_RGBA_Array.h"

PNG_RGBA_Array::PNG_RGBA_Array(unsigned long long int nPixels, unsigned int nBits) : _nBits(nBits), _nPixels(nPixels) {
    _data = new RGBA_Pixel[nPixels];
}

// Copy constructor
PNG_RGBA_Array::PNG_RGBA_Array(const PNG_RGBA_Array &source) {
    _data = nullptr;
    _nPixels = 0;
    operator=(source);
}


PNG_RGBA_Array::~PNG_RGBA_Array() {
    delete[] _data;
}

RGBA_Pixel &PNG_RGBA_Array::at(unsigned long long int n) {
    return _data[n];
}

RGBA_Pixel PNG_RGBA_Array::atC(unsigned long long int n) const {
    return _data[n];
}

unsigned int PNG_RGBA_Array::getDepthInBits() const noexcept {
    return _nBits;
}
