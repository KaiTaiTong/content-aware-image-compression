/**
 * @file PNG.cpp
 * @brief Implementation of modern PNG image handling
 * 
 * RAII-compliant PNG image operations with exception safety
 * and efficient memory management using smart pointers.
 */

#include "../../../include/utils/image/PNG.h"
#include "../../../include/utils/image/ColorConversion.h"
#include "../external/lodepng/lodepng.h"
#include <iostream>
#include <stdexcept>
#include <functional>

namespace ImageCompression {
namespace Utils {

PNG::PNG() : width_(0), height_(0), imageData_(nullptr) {
}

PNG::PNG(unsigned int width, unsigned int height) 
    : width_(width), height_(height) {
    if (width == 0 || height == 0) {
        throw std::invalid_argument("PNG dimensions must be positive");
    }
    
    size_t pixelCount = static_cast<size_t>(width_) * height_;
    imageData_ = std::make_unique<HSLAPixel[]>(pixelCount);
}

PNG::PNG(const PNG& other) : width_(other.width_), height_(other.height_) {
    copyFrom(other);
}

PNG::PNG(PNG&& other) noexcept 
    : width_(other.width_), height_(other.height_), imageData_(std::move(other.imageData_)) {
    other.width_ = 0;
    other.height_ = 0;
}

PNG::~PNG() = default;

PNG& PNG::operator=(const PNG& other) {
    if (this != &other) {
        width_ = other.width_;
        height_ = other.height_;
        copyFrom(other);
    }
    return *this;
}

PNG& PNG::operator=(PNG&& other) noexcept {
    if (this != &other) {
        width_ = other.width_;
        height_ = other.height_;
        imageData_ = std::move(other.imageData_);
        
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

bool PNG::operator==(const PNG& other) const {
    if (width_ != other.width_ || height_ != other.height_) {
        return false;
    }
    
    if (isEmpty() && other.isEmpty()) {
        return true;
    }
    
    size_t pixelCount = getPixelCount();
    for (size_t i = 0; i < pixelCount; ++i) {
        if (imageData_[i] != other.imageData_[i]) {
            return false;
        }
    }
    
    return true;
}

bool PNG::operator!=(const PNG& other) const {
    return !(*this == other);
}

bool PNG::loadFromFile(const std::string& filename) {
    std::vector<unsigned char> byteData;
    unsigned int width, height;
    
    unsigned error = lodepng::decode(byteData, width, height, filename);
    if (error) {
        throw std::runtime_error("PNG decode error " + std::to_string(error) + 
                               ": " + lodepng_error_text(error));
    }
    
    // Update dimensions and allocate new data
    width_ = width;
    height_ = height;
    size_t pixelCount = getPixelCount();
    imageData_ = std::make_unique<HSLAPixel[]>(pixelCount);
    
    // Convert RGB byte data to HSLA pixels
    for (size_t i = 0; i < byteData.size(); i += 4) {
        RGBColor rgb(byteData[i], byteData[i + 1], byteData[i + 2], byteData[i + 3]);
        HSLAColor hsla = rgbToHsla(rgb);
        
        HSLAPixel& pixel = imageData_[i / 4];
        pixel.hue = hsla.hue;
        pixel.saturation = hsla.saturation;
        pixel.luminance = hsla.luminance;
        pixel.alpha = hsla.alpha;
    }
    
    return true;
}

bool PNG::saveToFile(const std::string& filename) {
    if (isEmpty()) {
        throw std::runtime_error("Cannot save empty PNG image");
    }
    
    size_t pixelCount = getPixelCount();
    auto byteData = std::make_unique<unsigned char[]>(pixelCount * 4);
    
    // Convert HSLA pixels to RGB byte data
    for (size_t i = 0; i < pixelCount; ++i) {
        HSLAColor hsla(imageData_[i].hue, imageData_[i].saturation, 
                      imageData_[i].luminance, imageData_[i].alpha);
        RGBColor rgb = hslaToRgb(hsla);
        
        byteData[i * 4] = rgb.red;
        byteData[i * 4 + 1] = rgb.green;
        byteData[i * 4 + 2] = rgb.blue;
        byteData[i * 4 + 3] = rgb.alpha;
    }
    
    unsigned error = lodepng::encode(filename, byteData.get(), width_, height_);
    if (error) {
        throw std::runtime_error("PNG encode error " + std::to_string(error) + 
                               ": " + lodepng_error_text(error));
    }
    
    return true;
}

HSLAPixel* PNG::getPixel(unsigned int x, unsigned int y) {
    if (!isValidCoordinate(x, y)) {
        return nullptr;
    }
    
    size_t index = x + (static_cast<size_t>(y) * width_);
    return &imageData_[index];
}

const HSLAPixel* PNG::getPixel(unsigned int x, unsigned int y) const {
    if (!isValidCoordinate(x, y)) {
        return nullptr;
    }
    
    size_t index = x + (static_cast<size_t>(y) * width_);
    return &imageData_[index];
}

void PNG::resize(unsigned int newWidth, unsigned int newHeight) {
    if (newWidth == 0 || newHeight == 0) {
        throw std::invalid_argument("PNG dimensions must be positive");
    }
    
    size_t newPixelCount = static_cast<size_t>(newWidth) * newHeight;
    auto newImageData = std::make_unique<HSLAPixel[]>(newPixelCount);
    
    // Copy existing pixel data where it fits
    unsigned int minWidth = std::min(width_, newWidth);
    unsigned int minHeight = std::min(height_, newHeight);
    
    for (unsigned int y = 0; y < minHeight; ++y) {
        for (unsigned int x = 0; x < minWidth; ++x) {
            const HSLAPixel* oldPixel = getPixel(x, y);
            if (oldPixel) {
                size_t newIndex = x + (static_cast<size_t>(y) * newWidth);
                newImageData[newIndex] = *oldPixel;
            }
        }
    }
    
    // Update image properties
    width_ = newWidth;
    height_ = newHeight;
    imageData_ = std::move(newImageData);
}

std::size_t PNG::computeHash() const {
    if (isEmpty()) {
        return 0;
    }
    
    std::hash<double> hasher;
    std::size_t hash = 0;
    size_t pixelCount = getPixelCount();
    
    for (size_t i = 0; i < pixelCount; ++i) {
        const HSLAPixel& pixel = imageData_[i];
        hash ^= hasher(pixel.hue) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(pixel.saturation) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(pixel.luminance) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(pixel.alpha) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    
    return hash;
}

void PNG::normalizeColors() {
    if (isEmpty()) {
        return;
    }
    
    size_t pixelCount = getPixelCount();
    for (size_t i = 0; i < pixelCount; ++i) {
        HSLAPixel& pixel = imageData_[i];
        HSLAColor hsla(pixel.hue, pixel.saturation, pixel.luminance, pixel.alpha);
        normalizeHsla(hsla);
        
        pixel.hue = hsla.hue;
        pixel.saturation = hsla.saturation;
        pixel.luminance = hsla.luminance;
        pixel.alpha = hsla.alpha;
    }
}

void PNG::copyFrom(const PNG& other) {
    if (other.isEmpty()) {
        imageData_.reset();
        return;
    }
    
    size_t pixelCount = other.getPixelCount();
    imageData_ = std::make_unique<HSLAPixel[]>(pixelCount);
    
    for (size_t i = 0; i < pixelCount; ++i) {
        imageData_[i] = other.imageData_[i];
    }
}

bool PNG::isValidCoordinate(unsigned int x, unsigned int y) const {
    return x < width_ && y < height_ && !isEmpty();
}

std::ostream& operator<<(std::ostream& out, const PNG& image) {
    out << "PNG(" << image.getWidth() << "x" << image.getHeight() 
        << ", " << image.getPixelCount() << " pixels)";
    return out;
}

} // namespace Utils
} // namespace ImageCompression 