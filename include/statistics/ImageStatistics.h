#ifndef IMAGE_COMPRESSION_IMAGE_STATISTICS_H
#define IMAGE_COMPRESSION_IMAGE_STATISTICS_H

#include "../utils/image/PNG.h"
#include "../utils/image/HSLAPixel.h"
#include <utility>
#include <vector>
#include <cmath>

namespace ImageCompression {
    
    // A rectangular chunk of an image - defined by top-left and bottom-right corners
    struct Rectangle {
        std::pair<int, int> upperLeft;
        std::pair<int, int> lowerRight;
        
        Rectangle(std::pair<int, int> ul, std::pair<int, int> lr) 
            : upperLeft(ul), lowerRight(lr) {}
            
        Rectangle(int ulX, int ulY, int lrX, int lrY) 
            : upperLeft(std::make_pair(ulX, ulY)), lowerRight(std::make_pair(lrX, lrY)) {}
    };

    // Pre-calculates statistics for the image so we can quickly analyze any rectangular region
    // Uses cumulative sums - it's like having a lookup table for "what's the average color in this rectangle?"
    class ImageStatistics {
    public:
        static constexpr double PI = 3.14159265358979323846;
        static constexpr int HUE_BINS = 36;  // 360 degrees / 10 degrees per bin
        
        /**
         * @brief Constructs statistics for the given image
         * @param image The input image to analyze
         */
        explicit ImageStatistics(const Utils::PNG& image);
        
        /**
         * @brief Gets the average color for a rectangular region
         * @param region The rectangular region to analyze
         * @return Average HSLA pixel for the region
         */
        Utils::HSLAPixel getAverageColor(const Rectangle& region) const;
        
        /**
         * @brief Calculates the area (number of pixels) in a rectangle
         * @param region The rectangular region
         * @return Number of pixels in the region
         */
        long getArea(const Rectangle& region) const;
        
        /**
         * @brief Computes entropy for a rectangular region based on hue distribution
         * @param region The rectangular region to analyze
         * @return Entropy value for the region
         */
        double calculateEntropy(const Rectangle& region) const;
        
        /**
         * @brief Builds a hue histogram for a rectangular region
         * @param region The rectangular region
         * @return Vector containing hue frequency distribution
         */
        std::vector<int> buildHueHistogram(const Rectangle& region) const;
        
    private:
        // Cumulative sum arrays for efficient rectangle queries
        std::vector<std::vector<double>> cumulativeHueX_;  // cos(hue) values
        std::vector<std::vector<double>> cumulativeHueY_;  // sin(hue) values  
        std::vector<std::vector<double>> cumulativeSaturation_;
        std::vector<std::vector<double>> cumulativeLuminance_;
        
        // 3D array: [width][height][hue_bin] for hue histograms
        std::vector<std::vector<std::vector<int>>> cumulativeHueHistogram_;
        
        /**
         * @brief Helper function to subtract two histogram vectors
         * @param first First histogram
         * @param second Second histogram to subtract
         * @return Result of first - second
         */
        std::vector<int> subtractHistograms(const std::vector<int>& first, 
                                          const std::vector<int>& second) const;
        
        /**
         * @brief Helper function to add two histogram vectors
         * @param first First histogram
         * @param second Second histogram to add
         * @return Result of first + second
         */
        std::vector<int> addHistograms(const std::vector<int>& first, 
                                     const std::vector<int>& second) const;
        
        /**
         * @brief Calculates entropy from a frequency distribution
         * @param distribution Frequency distribution
         * @param totalArea Total number of elements
         * @return Entropy value
         */
        double calculateEntropyFromDistribution(const std::vector<int>& distribution, 
                                               int totalArea) const;
        
        /**
         * @brief Validates that a rectangle is within image bounds
         * @param region Rectangle to validate
         * @return true if valid, false otherwise
         */
        bool isValidRectangle(const Rectangle& region) const;
        
        // Image dimensions
        int imageWidth_;
        int imageHeight_;
    };

} // namespace ImageCompression

#endif // IMAGE_COMPRESSION_IMAGE_STATISTICS_H 