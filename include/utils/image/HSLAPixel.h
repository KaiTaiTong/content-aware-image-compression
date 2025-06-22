/**
 * @file HSLAPixel.h
 * @brief HSLA color space pixel representation for image processing
 * 
 * Modern C++17 implementation of HSLA (Hue, Saturation, Luminance, Alpha) 
 * color space pixel with efficient color operations and distance calculations.
 */

#pragma once

#include <iostream>
#include <sstream>

namespace ImageCompression {
namespace Utils {

/**
 * @brief HSLA color space pixel representation
 * 
 * Represents a pixel in HSLA color space with hue (0-360°), saturation (0-1),
 * luminance (0-1), and alpha (0-1) components. Provides efficient color
 * operations and perceptual distance calculations.
 */
class HSLAPixel {
public:
    double hue;        ///< Hue component in degrees [0, 360)
    double saturation; ///< Saturation component [0, 1]
    double luminance;  ///< Luminance component [0, 1] 
    double alpha;      ///< Alpha (transparency) component [0, 1]

    /**
     * @brief Default constructor - creates opaque white pixel
     */
    HSLAPixel();

    /**
     * @brief Construct opaque HSLA pixel with specified HSL values
     * @param h Hue in degrees [0, 360)
     * @param s Saturation [0, 1]
     * @param l Luminance [0, 1]
     */
    HSLAPixel(double h, double s, double l);

    /**
     * @brief Construct HSLA pixel with all components specified
     * @param h Hue in degrees [0, 360)
     * @param s Saturation [0, 1]
     * @param l Luminance [0, 1]
     * @param a Alpha [0, 1]
     */
    HSLAPixel(double h, double s, double l, double a);

    // Modern C++ copy semantics
    HSLAPixel(const HSLAPixel& other) = default;
    HSLAPixel& operator=(const HSLAPixel& other) = default;
    HSLAPixel(HSLAPixel&& other) = default;
    HSLAPixel& operator=(HSLAPixel&& other) = default;
    ~HSLAPixel() = default;

    // Comparison operators
    bool operator==(const HSLAPixel& other) const;
    bool operator!=(const HSLAPixel& other) const;
    bool operator<(const HSLAPixel& other) const;

    /**
     * @brief Calculate perceptual distance between two pixels
     * @param other The other pixel to compare with
     * @return Perceptual distance in HSLA color space
     */
    double distanceTo(const HSLAPixel& other) const;

    /**
     * @brief Check if two pixels are perceptually similar
     * @param other The other pixel to compare with
     * @param threshold Similarity threshold (default: 0.007)
     * @return True if pixels are similar within threshold
     */
    bool isSimilarTo(const HSLAPixel& other, double threshold = 0.007) const;
};

/**
 * @brief Stream output operator for HSLAPixel
 * @param out Output stream
 * @param pixel Pixel to output
 * @return Reference to output stream
 */
std::ostream& operator<<(std::ostream& out, const HSLAPixel& pixel);

/**
 * @brief String stream output operator for HSLAPixel
 * @param out String stream
 * @param pixel Pixel to output  
 * @return Reference to string stream
 */
std::stringstream& operator<<(std::stringstream& out, const HSLAPixel& pixel);

} // namespace Utils
} // namespace ImageCompression 