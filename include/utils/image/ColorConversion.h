/**
 * @file ColorConversion.h
 * @brief Modern color space conversion utilities
 * 
 * High-performance RGB ↔ HSLA color space conversions with
 * proper handling of edge cases and perceptual accuracy.
 */

#pragma once

#include <cstdint>

namespace ImageCompression {
namespace Utils {

/**
 * @brief RGB color representation
 */
struct RGBColor {
    uint8_t red;   ///< Red component [0-255]
    uint8_t green; ///< Green component [0-255] 
    uint8_t blue;  ///< Blue component [0-255]
    uint8_t alpha; ///< Alpha component [0-255]
    
    RGBColor() : red(0), green(0), blue(0), alpha(255) {}
    RGBColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) 
        : red(r), green(g), blue(b), alpha(a) {}
};

/**
 * @brief HSLA color representation
 */
struct HSLAColor {
    double hue;        ///< Hue in degrees [0-360)
    double saturation; ///< Saturation [0-1]
    double luminance;  ///< Luminance [0-1]
    double alpha;      ///< Alpha [0-1]
    
    HSLAColor() : hue(0.0), saturation(0.0), luminance(1.0), alpha(1.0) {}
    HSLAColor(double h, double s, double l, double a = 1.0) 
        : hue(h), saturation(s), luminance(l), alpha(a) {}
};

/**
 * @brief Convert RGB color to HSLA color space
 * @param rgb RGB color to convert
 * @return Equivalent HSLA color
 */
HSLAColor rgbToHsla(const RGBColor& rgb);

/**
 * @brief Convert HSLA color to RGB color space
 * @param hsla HSLA color to convert
 * @return Equivalent RGB color
 */
RGBColor hslaToRgb(const HSLAColor& hsla);

/**
 * @brief Normalize HSLA values to valid ranges
 * @param hsla HSLA color to normalize (modified in place)
 */
void normalizeHsla(HSLAColor& hsla);

/**
 * @brief Clamp RGB values to valid ranges
 * @param rgb RGB color to clamp (modified in place)
 */
void clampRgb(RGBColor& rgb);

} // namespace Utils
} // namespace ImageCompression 