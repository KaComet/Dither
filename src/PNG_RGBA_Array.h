#ifndef DITHER_PNG_RGBA_ARRAY_H
#define DITHER_PNG_RGBA_ARRAY_H

#include "PNG_structs.h"

/* Essentially an array with added functions that allow for easy copying. */
class PNG_RGBA_Array {
public:
    explicit PNG_RGBA_Array(unsigned long long nPixels, unsigned int nBits);
    PNG_RGBA_Array(const PNG_RGBA_Array &source);

    ~PNG_RGBA_Array();

    // Returns element at n.
    RGBA_Pixel &at(unsigned long long n);

    // Returns element at n.
    [[nodiscard]] RGBA_Pixel atC(unsigned long long n) const;

    // Assignment operator
    PNG_RGBA_Array &operator=(const PNG_RGBA_Array &other) {
        /* If the source and destination are the same, do nothing.
         *   This is also necessary to avoid a seg-fault */
        if ((_data != other._data) || (this == &other)) {
            // Delete the old data
            delete[] _data;

            // Set the number of pixel and create the new array.
            _nPixels = other._nPixels;
            _nBits = other._nBits;
            _data = new RGBA_Pixel[_nPixels];

            // Transfer the contents from the source array to the destination array.
            for (unsigned long long i = 0; i < _nPixels; i++)
                _data[i] = other._data[i];
        }

        return *this;
    }

    [[nodiscard]] unsigned int getDepthInBits() const noexcept;

protected:
    RGBA_Pixel *_data; // Data array
    unsigned int _nBits;
    unsigned long long _nPixels;
};


#endif //DITHER_PNG_RGBA_ARRAY_H
