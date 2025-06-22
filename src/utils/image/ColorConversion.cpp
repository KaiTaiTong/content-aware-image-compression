/**
 * @file ColorConversion.cpp
 * @brief Implementation of color space conversion utilities
 * 
 * High-performance RGB ↔ HSLA conversions with proper handling
 * of edge cases and numerical stability.
 */

#include "../../../include/utils/image/ColorConversion.h"
#include <algorithm>
#include <cmath>

namespace ImageCompression {
namespace Utils {

namespace {
    constexpr double EPSILON = 1e-10;
    constexpr double ONE_THIRD = 1.0 / 3.0;
    constexpr double TWO_THIRDS = 2.0 / 3.0;
    constexpr double ONE_SIXTH = 1.0 / 6.0;
}

HSLAColor rgbToHsla(const RGBColor& rgb) {
    // Convert RGB [0-255] to normalized [0-1] range
    double r = rgb.red / 255.0;
    double g = rgb.green / 255.0;
    double b = rgb.blue / 255.0;
    double a = rgb.alpha / 255.0;
    
    double maxVal = std::max({r, g, b});
    double minVal = std::min({r, g, b});
    double delta = maxVal - minVal;
    
    HSLAColor hsla;
    hsla.alpha = a;
    
    // Calculate luminance
    hsla.luminance = (maxVal + minVal) * 0.5;
    
    // Handle grayscale case
    if (delta < EPSILON) {
        hsla.hue = 0.0;
        hsla.saturation = 0.0;
        return hsla;
    }
    
    // Calculate saturation
    if (hsla.luminance < 0.5) {
        hsla.saturation = delta / (maxVal + minVal);
    } else {
        hsla.saturation = delta / (2.0 - maxVal - minVal);
    }
    
    // Calculate hue
    if (maxVal == r) {
        hsla.hue = ((g - b) / delta);
        if (g < b) hsla.hue += 6.0;
    } else if (maxVal == g) {
        hsla.hue = ((b - r) / delta) + 2.0;
    } else {
        hsla.hue = ((r - g) / delta) + 4.0;
    }
    
    hsla.hue *= 60.0; // Convert to degrees
    
    return hsla;
}

namespace {
    double hueToRgb(double p, double q, double t) {
        if (t < 0.0) t += 1.0;
        if (t > 1.0) t -= 1.0;
        if (t < ONE_SIXTH) return p + (q - p) * 6.0 * t;
        if (t < 0.5) return q;
        if (t < TWO_THIRDS) return p + (q - p) * (TWO_THIRDS - t) * 6.0;
        return p;
    }
}

RGBColor hslaToRgb(const HSLAColor& hsla) {
    RGBColor rgb;
    rgb.alpha = static_cast<uint8_t>(std::round(hsla.alpha * 255.0));
    
    // Handle grayscale case
    if (hsla.saturation < EPSILON) {
        uint8_t gray = static_cast<uint8_t>(std::round(hsla.luminance * 255.0));
        rgb.red = rgb.green = rgb.blue = gray;
        return rgb;
    }
    
    double h = hsla.hue / 360.0; // Normalize hue to [0-1]
    double s = hsla.saturation;
    double l = hsla.luminance;
    
    double q = (l < 0.5) ? l * (1.0 + s) : l + s - l * s;
    double p = 2.0 * l - q;
    
    double r = hueToRgb(p, q, h + ONE_THIRD);
    double g = hueToRgb(p, q, h);
    double b = hueToRgb(p, q, h - ONE_THIRD);
    
    rgb.red = static_cast<uint8_t>(std::round(r * 255.0));
    rgb.green = static_cast<uint8_t>(std::round(g * 255.0));
    rgb.blue = static_cast<uint8_t>(std::round(b * 255.0));
    
    return rgb;
}

void normalizeHsla(HSLAColor& hsla) {
    // Normalize hue to [0, 360) range
    hsla.hue = std::fmod(hsla.hue, 360.0);
    if (hsla.hue < 0.0) hsla.hue += 360.0;
    
    // Clamp saturation, luminance, and alpha to [0, 1]
    hsla.saturation = std::max(0.0, std::min(1.0, hsla.saturation));
    hsla.luminance = std::max(0.0, std::min(1.0, hsla.luminance));
    hsla.alpha = std::max(0.0, std::min(1.0, hsla.alpha));
}

void clampRgb(RGBColor& rgb) {
    // RGB values are already constrained by uint8_t type
    // This function is provided for completeness and future extensibility
}

} // namespace Utils
} // namespace ImageCompression 