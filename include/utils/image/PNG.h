/**
 * @file PNG.h
 * @brief Modern PNG image handling for content-aware compression
 * 
 * Professional C++17 implementation of PNG image loading, saving, and
 * manipulation using HSLA color space for perceptual accuracy.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "HSLAPixel.h"

namespace ImageCompression {
namespace Utils {

/**
 * @brief High-performance PNG image container with HSLA pixel support
 * 
 * Modern C++17 implementation providing efficient PNG image operations
 * with RAII memory management and exception-safe operations.
 */
class PNG {
public:
    /**
     * @brief Default constructor - creates empty image
     */
    PNG();

    /**
     * @brief Construct PNG with specified dimensions
     * @param width Image width in pixels
     * @param height Image height in pixels
     */
    PNG(unsigned int width, unsigned int height);

    /**
     * @brief Copy constructor
     * @param other PNG image to copy
     */
    PNG(const PNG& other);

    /**
     * @brief Move constructor
     * @param other PNG image to move from
     */
    PNG(PNG&& other) noexcept;

    /**
     * @brief Destructor - automatically manages memory
     */
    ~PNG();

    /**
     * @brief Copy assignment operator
     * @param other PNG image to copy
     * @return Reference to this image
     */
    PNG& operator=(const PNG& other);

    /**
     * @brief Move assignment operator
     * @param other PNG image to move from
     * @return Reference to this image
     */
    PNG& operator=(PNG&& other) noexcept;

    /**
     * @brief Equality comparison operator
     * @param other PNG image to compare with
     * @return True if images are identical
     */
    bool operator==(const PNG& other) const;

    /**
     * @brief Inequality comparison operator
     * @param other PNG image to compare with
     * @return True if images are different
     */
    bool operator!=(const PNG& other) const;

    /**
     * @brief Load PNG image from file
     * @param filename Path to PNG file
     * @return True if successfully loaded
     * @throws std::runtime_error if file cannot be loaded
     */
    bool loadFromFile(const std::string& filename);

    /**
     * @brief Save PNG image to file
     * @param filename Path to save PNG file
     * @return True if successfully saved
     * @throws std::runtime_error if file cannot be saved
     */
    bool saveToFile(const std::string& filename);

    /**
     * @brief Get pixel at specified coordinates
     * @param x X coordinate (0 = leftmost)
     * @param y Y coordinate (0 = topmost)
     * @return Pointer to pixel (nullptr if out of bounds)
     */
    HSLAPixel* getPixel(unsigned int x, unsigned int y);

    /**
     * @brief Get const pixel at specified coordinates
     * @param x X coordinate (0 = leftmost)
     * @param y Y coordinate (0 = topmost)
     * @return Const pointer to pixel (nullptr if out of bounds)
     */
    const HSLAPixel* getPixel(unsigned int x, unsigned int y) const;

    /**
     * @brief Get image width
     * @return Width in pixels
     */
    unsigned int getWidth() const { return width_; }

    /**
     * @brief Get image height
     * @return Height in pixels
     */
    unsigned int getHeight() const { return height_; }

    /**
     * @brief Check if image is empty
     * @return True if image has no pixels
     */
    bool isEmpty() const { return width_ == 0 || height_ == 0; }

    /**
     * @brief Get total number of pixels
     * @return Total pixel count
     */
    size_t getPixelCount() const { return static_cast<size_t>(width_) * height_; }

    /**
     * @brief Resize image (crops or pads as needed)
     * @param newWidth New width in pixels
     * @param newHeight New height in pixels
     */
    void resize(unsigned int newWidth, unsigned int newHeight);

    /**
     * @brief Compute hash of image contents for comparison
     * @return Hash value of image data
     */
    std::size_t computeHash() const;

    /**
     * @brief Apply color space normalization
     * 
     * Ensures colors are within valid ranges and applies any necessary
     * color space conversions for consistency.
     */
    void normalizeColors();

private:
    unsigned int width_;                           ///< Image width in pixels
    unsigned int height_;                          ///< Image height in pixels
    std::unique_ptr<HSLAPixel[]> imageData_;      ///< Pixel data array
    
    /**
     * @brief Copy pixel data from another PNG
     * @param other Source PNG to copy from
     */
    void copyFrom(const PNG& other);

    /**
     * @brief Validate coordinates are within image bounds
     * @param x X coordinate to check
     * @param y Y coordinate to check
     * @return True if coordinates are valid
     */
    bool isValidCoordinate(unsigned int x, unsigned int y) const;
};

/**
 * @brief Stream output operator for PNG images
 * @param out Output stream
 * @param image PNG image to output
 * @return Reference to output stream
 */
std::ostream& operator<<(std::ostream& out, const PNG& image);

} // namespace Utils
} // namespace ImageCompression 