#ifndef DITHER_PNG_RGB_ARRAY_H
#define DITHER_PNG_RGB_ARRAY_H

#include "PNG_structs.h"

/* Essentially an array with added functions that allow for easy copying. */
class PNG_RGB_Array {
public:
    explicit PNG_RGB_Array(unsigned long long nPixels);
    PNG_RGB_Array(const PNG_RGB_Array &source);

    ~PNG_RGB_Array();

    // Returns element at n.
    RGBA_Pixel &at(unsigned long long n);

    // Assignment operator
    PNG_RGB_Array &operator=(const PNG_RGB_Array &other) {
        /* If the source and destination are the same, do nothing.
         *   This is also necessary to avoid a seg-fault */
        if ((_data != other._data) || (this == &other)) {
            // Delete the old data
            delete[] _data;

            // Set the number of pixel and create the new array.
            _nPixels = other._nPixels;
            _data = new RGBA_Pixel[_nPixels];

            // Transfer the contents from the source array to the destination array.
            for (unsigned long long i = 0; i < _nPixels; i++)
                _data[i] = other._data[i];
        }

        return *this;
    }

protected:
    RGBA_Pixel *_data; // Data array
    unsigned long long _nPixels;
};


#endif //DITHER_PNG_RGB_ARRAY_H
