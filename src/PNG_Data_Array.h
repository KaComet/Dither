#ifndef DITHER_PNG_DATA_ARRAY_H
#define DITHER_PNG_DATA_ARRAY_H

#include "PNG_structs.h"

/* Essentially an array with added functions that allow for easy copying. */
template <typename T>
class PNG_Data_Array {
public:
    explicit PNG_Data_Array(unsigned long long nPixels, unsigned int nBits) : _nBits(nBits), _nPixels(nPixels) {
        _data = new T[nPixels];
    };

    PNG_Data_Array(const PNG_Data_Array<T> &source) {
        _data = nullptr;
        _nPixels = 0;
        _nBits = 0;
        operator=(source);
    };

    ~PNG_Data_Array() {
        delete[] _data;
    };

    // Returns element at n.
    T &at(unsigned long long n) {
        return _data[n];
    };

    // Returns element at n.
    [[nodiscard]] T atC(unsigned long long n) const {
        return _data[n];
    };

    // Assignment operator
    PNG_Data_Array<T> &operator=(const PNG_Data_Array<T> &other) {
        /* If the source and destination are the same, do nothing.
         *   This is also necessary to avoid a seg-fault */
        if ((_data != other._data) || (this == &other)) {
            // Delete the old data
            delete[] _data;

            // Set the number of pixel and create the new array.
            _nPixels = other._nPixels;
            _nBits = other._nBits;
            _data = new T[_nPixels];

            // Transfer the contents from the source array to the destination array.
            for (unsigned long long i = 0; i < _nPixels; i++)
                _data[i] = other._data[i];
        }

        return *this;
    }

    [[nodiscard]] unsigned int getDepthInBits() const noexcept {
        return _nBits;
    };

protected:
    T *_data; // Data array
    unsigned int _nBits;
    unsigned long long _nPixels;
};


#endif //DITHER_PNG_DATA_ARRAY_H
