#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <png.h>
#include <cmath>
#include "PNG_Loader.h"
#include "PNG_RGBA.h"
#include "PNG_structs.h"

const unsigned char bayer4X4[4][4] = {{0,  8,  2,  10},
                                      {12, 4,  14, 6},
                                      {3,  11, 1,  9},
                                      {15, 7,  13, 5}};

PNG_RGBA bayerRGBA(PNG_RGBA &input, const unsigned char map[][4], unsigned char mapWidth, bool using3Bit);

template<typename T>
T pixelToGrey(T red, T blue, T green);

HSV_Color RGB_PixelToHSV_Color(RGB_Pixel rgb);

int main(int argc, char *argv[]) {
    std::string inputFilePath;
    std::string outputFilePath;
    bool skip = false;
    bool using3Bit = false;
    bool modeSet = false;

    //Process the input arguments
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

    // Load the PNG. If the format or bit depth is not supported, exit.
    PNG_Info fileInfo = PNG_Loader::IdentifyPNG(inputFilePath);
    if (((fileInfo.colorType != PNG_ColorType::RGBA) && (fileInfo.colorType != PNG_ColorType::RGB_truecolor) &&
         (fileInfo.colorType != PNG_ColorType::grayscale) && (fileInfo.colorType != PNG_ColorType::grayscale_alpha) &&
         (fileInfo.colorDepth != 8) & (fileInfo.colorDepth != 16))) {
        std::cout << "File is not compatible.\n";
        return 1;
    }
    auto png = PNG_RGBA(inputFilePath, true);

    // Perform Bayer Dithering on the image using the color mode specified.
    png = bayerRGBA(png, bayer4X4, 4, using3Bit);

    // Write the resultant PNG.
    png.write_png_file(outputFilePath);

    return 0;
}

PNG_RGBA bayerRGBA(PNG_RGBA &input, const unsigned char map[][4], unsigned char mapWidth, bool using3Bit) {
    PNG_RGBA resultPNG = input;

    // Scan through every pixel in the image.
    for (png_uint_32 y = 0; y < input.getInfo().height; y += mapWidth) {
        for (png_uint_32 x = 0; x < input.getInfo().width; x += mapWidth) {

            // Scan through each pixel of the Bayer map.
            for (unsigned long int m_y = 0; m_y < mapWidth; m_y++) {
                // If the pixel is outside the image, continue.
                if ((m_y + y) >= input.getInfo().height)
                    continue;

                for (unsigned long int m_x = 0; m_x < mapWidth; m_x++) {
                    // If the pixel is outside the image, continue.
                    if ((m_x + x) >= input.getInfo().width)
                        continue;

                    // Get the pixel at the calculated location.
                    std::optional<RGBA_Pixel> pixelOpt = input.getPixel(x + m_x, y + m_y);

                    // NULL check
                    if (!pixelOpt.has_value())
                        continue;

                    // The default value of the result is a black pixel.
                    auto resultPixel = RGBA_Pixel{0x00, 0x00, 0x00, 0xFF};
                    RGBA_Pixel pixel = pixelOpt.value();

                    // If using 3Bit color mode.
                    if (using3Bit) {
                        // If the color red exceeds the threshold, fill it in.
                        if ((((double) pixel.red) / (255.0) > (((double) map[m_x][m_y]) / (16.0))))
                            resultPixel.red += 0xFF;

                        // If the color blue exceeds the threshold, fill it in.
                        if ((((double) pixel.blue) / (255.0) > (((double) map[m_x][m_y]) / (16.0))))
                            resultPixel.blue += 0xFF;

                        // If the color green exceeds the threshold, fill it in.
                        if ((((double) pixel.green) / (255.0) > (((double) map[m_x][m_y]) / (16.0))))
                            resultPixel.green += 0xFF;
                    } else { // If using greyscale mode.
                        // Convert the pixel to greyscale.
                        png_byte grey = pixelToGrey(pixel.red, pixel.blue, pixel.green);

                        // If the pixel's value exceeds the threshold, fill it in.
                        if ((((double) grey) / (255.0) > (((double) map[m_x][m_y]) / (16.0))))
                            resultPixel = RGBA_Pixel{0xFF, 0xFF, 0xFF, 255};
                    }

                    // Save the resultant pixel to the output PNG.
                    resultPNG.setPixel(m_x + x, m_y + y, resultPixel);
                }
            }
        }
    }

    return resultPNG;
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

