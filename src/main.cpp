#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <png.h>
#include <cmath>
#include "PNG_Loader.h"
#include "PNG_RGBA.h"
#include "PNG_Grey.h"
#include "PNG_structs.h"

const double bayer4X4[4][4] = {{0,  8,  2,  10},
                               {12, 4,  14, 6},
                               {3,  11, 1,  9},
                               {15, 7,  13, 5}};

PNG_RGBA bayerRGBA(PNG_RGBA &input, const double map[][4], unsigned int maxValue);

PNG_Grey bayerGrey(PNG_RGBA &input, const double map[][4], unsigned int maxValue);

bool exceedsThreshold(double value, double maxValue, double threshold, double thresholdDivisor);

template<typename T>
T pixelToGrey(T red, T blue, T green);

HSV_Color RGB_PixelToHSV_Color(RGB_Pixel rgb);

void
processInputArgs(int argc, char *argv[], std::string &inputFilePath, std::string &outputFilePath, bool &using3Bit);

int main(int argc, char *argv[]) {
    std::string inputFilePath;
    std::string outputFilePath;
    bool using3Bit = false;

    processInputArgs(argc, argv, inputFilePath, outputFilePath, using3Bit);


    // Load the PNG. If the format or bit depth is not supported, exit.
    PNG_Info fileInfo{};
    try {
        fileInfo = PNG_Loader::IdentifyPNG(inputFilePath);
    } catch (BadPath &e) {
        std::cout << "Could not load file at source. Aborting." << std::endl;
        exit(1);
    } catch (NotPNG &e) {
        std::cout << "File is not a PNG. Aborting" << std::endl;
        exit(1);
    } catch (std::runtime_error &e) {
        std::cout << "Fatal error. Program threw the following exception: " << e.what() << std::endl;
        exit(1);
    } catch (UnsupportedColorMode &e) {
        std::cout << "File color mode not supported. Aborting." << std::endl;
        exit(1);
    }
    if (((fileInfo.colorType != PNG_ColorType::RGBA) && (fileInfo.colorType != PNG_ColorType::RGB_truecolor) &&
         (fileInfo.colorType != PNG_ColorType::grayscale) && (fileInfo.colorType != PNG_ColorType::grayscale_alpha) &&
         (fileInfo.colorDepth != 8) & (fileInfo.colorDepth != 16))) {
        std::cout << "File is not compatible.\n";
        return 1;
    }

    PNG_RGBA png;
    try {
        png = PNG_RGBA(inputFilePath);
    } catch (BadPath &e) {
        std::cout << "Could not load file at source. Aborting." << std::endl;
        exit(1);
    } catch (NotPNG &e) {
        std::cout << "File is not a PNG. Aborting" << std::endl;
        exit(1);
    } catch (std::runtime_error &e) {
        std::cout << "Fatal error. Program threw the following exception: " << e.what() << std::endl;
        exit(1);
    } catch (UnsupportedColorMode &e) {
        std::cout << "File color mode not supported. Aborting." << std::endl;
        exit(1);
    }

    // Perform Bayer Dithering on the image using the color mode specified.
    if (using3Bit) {
        png = bayerRGBA(png, bayer4X4, pow(2, png.getInfo().colorDepth) - 1);

        // Write the resultant PNG.
        try {
            png.write_png_file(outputFilePath);
        } catch (BadPath &e) {
            std::cout << "Could not create file at destination. Aborting." << std::endl;
            exit(1);
        } catch (std::runtime_error &e) {
            std::cout << "Fatal error. Program threw the following exception: " << e.what() << std::endl;
            exit(1);
        }
    } else {
        PNG_Grey pngGrey = bayerGrey(png, bayer4X4, pow(2, png.getInfo().colorDepth) - 1);

        // Write the resultant PNG.
        try {
            pngGrey.write_png_file(outputFilePath);
        } catch (BadPath &e) {
            std::cout << "Could not create file at destination. Aborting." << std::endl;
            exit(1);
        } catch (std::runtime_error &e) {
            std::cout << "Fatal error. Program threw the following exception: " << e.what() << std::endl;
            exit(1);
        }
    }

    return 0;
}

PNG_RGBA bayerRGBA(PNG_RGBA &input, const double map[][4], unsigned int maxValue) {
    PNG_RGBA resultPNG = input;

    // Scan through every pixel in the image.
    for (png_uint_32 y = 0; y < resultPNG.getInfo().height; y++) {
        for (png_uint_32 x = 0; x < resultPNG.getInfo().width; x++) {
            // Get the pixel at the calculated location.
            RGBA_Pixel pixel = input.getPixel(x, y).value();

            // The default value of the result is a black pixel.
            auto resultPixel = RGBA_Pixel{0x00, 0x00, 0x00, maxValue};

            // If the color red exceeds the threshold, fill it in.
            if (exceedsThreshold(pixel.red, maxValue, map[x % 4][y % 4], 16.0))
                resultPixel.red = maxValue;

            // If the color blue exceeds the threshold, fill it in.
            if (exceedsThreshold(pixel.blue, maxValue, map[x % 4][y % 4], 16.0))
                resultPixel.blue = maxValue;

            // If the color green exceeds the threshold, fill it in.
            if (exceedsThreshold(pixel.green, maxValue, map[x % 4][y % 4], 16.0))
                resultPixel.green = maxValue;

            // Save the resultant pixel to the output PNG.
            resultPNG.setPixel(x, y, resultPixel);
        }
    }

    return resultPNG;
}

PNG_Grey bayerGrey(PNG_RGBA &input, const double map[][4], unsigned int maxValue) {
    PNG_Grey resultPNG = PNG_Grey(input.getInfo().width, input.getInfo().height, 8);

    // Scan through every pixel in the image.
    for (png_uint_32 y = 0; y < resultPNG.getInfo().height; y++) {
        for (png_uint_32 x = 0; x < resultPNG.getInfo().width; x++) {
            // Get the pixel at the calculated location.
            RGBA_Pixel pixel = input.getPixel(x, y).value();

            // The default value of the result is a black pixel.
            GreyPixel resultPixel = 0;

            // If using 3Bit color mode.
            // Convert the pixel to greyscale.
            unsigned int grey = pixelToGrey(pixel.red, pixel.blue, pixel.green);

            // If the pixel's value exceeds the threshold, fill it in.
            if (exceedsThreshold(grey, maxValue, map[x % 4][y % 4], 16.0))
                resultPixel = 255;

            // Save the resultant pixel to the output PNG.
            resultPNG.setPixel(x, y, resultPixel);
        }
    }

    return resultPNG;
}

bool exceedsThreshold(double value, double maxValue, double threshold, double thresholdDivisor) {
    return (value / maxValue) > (threshold / thresholdDivisor);
}

/* Converts a color pixel to greyscale.
 *   Colors are weighted by luminosity */
template<typename T>
T pixelToGrey(T red, T blue, T green) {
    return ((0.21 * (double) red) + (0.72 * (double) green) + (0.07 * (double) blue));
}

// Converts a RGB pixel to a HSV pixel.
HSV_Color RGB_PixelToHSV_Color(RGB_Pixel rgb) {
    double tempH = 0, tempS, tempV;
    double r = (double) rgb.red / 255.0;
    double g = (double) rgb.green / 255.0;
    double b = (double) rgb.blue / 255.0;
    double cMax = fmax(fmax(r, g), b);
    double cMin = fmin(fmin(r, g), b);
    double fD = cMax - cMin;

    if (fD > 0) {
        if (cMax == r) {
            tempH = 60 * (fmod(((g - b) / fD), 6));
        } else if (cMax == g) {
            tempH = 60 * (((b - r) / fD) + 2);
        } else if (cMax == b) {
            tempH = 60 * (((r - g) / fD) + 4);
        }

        tempV = cMax;
        tempS = 0;
        if (cMax > 0) {
            tempS = fD / cMax;
        }
    } else {
        tempH = 0;
        tempS = 0;
        tempV = cMax;
    }

    if (tempH < 0) {
        tempH = 360 + tempH;
    }

    auto h = (unsigned int) round(tempH * 360);
    auto s = (unsigned int) round(tempS * 100);
    auto v = (unsigned int) round(tempV * 100);

    return HSV_Color{h, s, v};
}

void processInputArgs(int argc, char *argv[], std::string &inputFilePath,
                      std::string &outputFilePath, bool &using3Bit) {
    //Process the input arguments
    bool modeSet = false;
    bool skip = false;
    for (int i = 1; i < argc; i++) {
        // Skip this argument if necessary.
        if (skip) {
            skip = false;
            continue;
        }
        // Convert the argument to a string
        std::string argument = std::string(argv[i]);

        // If the argument was "--help", print the help screen and exit.
        if (argument == "--help") {
            std::cout << "Usage : dither [Input Path]... [Output Path]... [Options]...\n"
                      << "Dithers a PNG file\n"
                      << "\n"
                      << "  -m                    sets the dithering color mode(greyscale or 3bit). Default is greyscale\n";
            exit(0);
        }

        // If the argument was "-m", ensure that the mode has not already been set. If not, exit.
        if (argument == "-m") {
            if (modeSet) {
                std::cout << "Operation \"-m\" cannot be defined twice.\nTry 'dither --help' for more information.\n";
                exit(1);
            }

            // Ensure that there is an argument following "-m". If not, exit.
            if (i == (argc - 1)) {
                std::cout << "Operation \"-m\" requires argument.\nTry 'dither --help' for more information.\n";
                exit(1);
            }

            // If the argument following "-m" is another command argument, exit.
            std::string argument2 = std::string(argv[i + 1]);
            if (argument2.at(0) == '-') {
                std::cout << "Operation \"-m\" requires argument.\nTry 'dither --help' for more information.\n";
                exit(1);
            }

            /* If all the previous checks has been passed, check if the supplied argument is
             *   one of the valid argument. If so, load. If not, exit. */
            if (argument2 == "3bit") {
                modeSet = true;
                using3Bit = true;
                skip = true;
                continue;
            } else if (argument2 == "greyscale") {
                modeSet = true;
                using3Bit = false;
                skip = true;
                continue;
            } else {
                std::cout << '\"' << argument2
                          << "\" not recognized as a valid mode.\nTry 'dither --help' for more information.\n";
                exit(1);
            }
        }

        /* If the argument is not a command, check if the input path has
         *   been set. If not, load and check the next argument. */
        if (inputFilePath.empty()) {
            inputFilePath = argument;
            continue;
        }

        /* Check if the output path has been set. If not, load
         *   and check the next argument. */
        if ((!inputFilePath.empty()) && (outputFilePath.empty())) {
            outputFilePath = argument;
            continue;
        }

        // If too many arguments have been supplied, exit.
        if ((!inputFilePath.empty()) && (!outputFilePath.empty())) {
            std::cout << "Too many operands provided\nTry 'dither --help' for more information.\n";
            exit(1);
        }
    }

    // Ensure that both the input and output paths have been set.
    if (inputFilePath.empty()) {
        std::cout << "Missing input file path\nTry 'dither --help' for more information.\n";
        exit(1);
    }
    if (outputFilePath.empty()) {
        std::cout << "Missing output file path\nTry 'dither --help' for more information.\n";
        exit(1);
    }
}