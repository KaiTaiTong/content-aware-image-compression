/**
 * @file HSLAPixel.cpp
 * @brief Implementation of HSLA color space pixel operations
 * 
 * Modern C++17 implementation providing efficient color operations
 * and perceptual distance calculations in HSLA color space.
 */

#include "../../../include/utils/image/HSLAPixel.h"
#include <cmath>
#include <iostream>

namespace ImageCompression {
namespace Utils {

namespace {
    constexpr double PI = 3.14159265358979323846;
    constexpr double SIMILARITY_THRESHOLD = 0.007;
    constexpr double EPSILON = 1e-9;
}

HSLAPixel::HSLAPixel() 
    : hue(0.0), saturation(0.0), luminance(1.0), alpha(1.0) {
}

HSLAPixel::HSLAPixel(double h, double s, double l) 
    : hue(h), saturation(s), luminance(l), alpha(1.0) {
}

HSLAPixel::HSLAPixel(double h, double s, double l, double a) 
    : hue(h), saturation(s), luminance(l), alpha(a) {
}

bool HSLAPixel::operator==(const HSLAPixel& other) const {
    return isSimilarTo(other, SIMILARITY_THRESHOLD);
}

bool HSLAPixel::operator!=(const HSLAPixel& other) const {
    return !(*this == other);
}

bool HSLAPixel::operator<(const HSLAPixel& other) const {
    if (*this == other) return false;

    // Compare by luminance first (most perceptually significant)
    if (std::abs(luminance - other.luminance) > EPSILON) {
        return luminance < other.luminance;
    }

    // Then by saturation
    if (std::abs(saturation - other.saturation) > EPSILON) {
        return saturation < other.saturation;
    }

    // Then by hue
    if (std::abs(hue - other.hue) > EPSILON) {
        return hue < other.hue;
    }

    // Finally by alpha
    return alpha < other.alpha;
}

double HSLAPixel::distanceTo(const HSLAPixel& other) const {
    // Convert to cylindrical coordinates and compute Euclidean distance
    // This gives perceptually meaningful distance in HSLA space
    
    double h1_rad = hue * PI / 180.0;
    double h2_rad = other.hue * PI / 180.0;
    
    // Project onto color cone using saturation and luminance
    double x1 = std::sin(h1_rad) * saturation * luminance;
    double y1 = std::cos(h1_rad) * saturation * luminance;
    double z1 = luminance;
    
    double x2 = std::sin(h2_rad) * other.saturation * other.luminance;
    double y2 = std::cos(h2_rad) * other.saturation * other.luminance;
    double z2 = other.luminance;
    
    // Euclidean distance in 3D color space
    double dx = x1 - x2;
    double dy = y1 - y2;
    double dz = z1 - z2;
    
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

bool HSLAPixel::isSimilarTo(const HSLAPixel& other, double threshold) const {
    return distanceTo(other) < threshold;
}

std::ostream& operator<<(std::ostream& out, const HSLAPixel& pixel) {
    out << "HSLA(" << pixel.hue << "°, " 
        << (pixel.saturation * 100) << "%, " 
        << (pixel.luminance * 100) << "%";
    
    if (pixel.alpha != 1.0) {
        out << ", " << pixel.alpha;
    }
    
    out << ")";
    return out;
}

std::stringstream& operator<<(std::stringstream& out, const HSLAPixel& pixel) {
    out << "HSLA(" << pixel.hue << "°, " 
        << (pixel.saturation * 100) << "%, " 
        << (pixel.luminance * 100) << "%";
    
    if (pixel.alpha != 1.0) {
        out << ", " << pixel.alpha;
    }
    
    out << ")";
    return out;
}

} // namespace Utils
} // namespace ImageCompression 