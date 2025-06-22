#include "../../include/statistics/ImageStatistics.h"
#include <cmath>
#include <algorithm>
#include <cassert>

namespace ImageCompression {

    ImageStatistics::ImageStatistics(const Utils::PNG& image) 
        : imageWidth_(image.getWidth()), imageHeight_(image.getHeight()) {
        
        // Initialize cumulative arrays
        cumulativeHueX_.resize(imageWidth_);
        cumulativeHueY_.resize(imageWidth_);
        cumulativeSaturation_.resize(imageWidth_);
        cumulativeLuminance_.resize(imageWidth_);
        cumulativeHueHistogram_.resize(imageWidth_);
        
        for (int x = 0; x < imageWidth_; ++x) {
            cumulativeHueX_[x].resize(imageHeight_);
            cumulativeHueY_[x].resize(imageHeight_);
            cumulativeSaturation_[x].resize(imageHeight_);
            cumulativeLuminance_[x].resize(imageHeight_);
            cumulativeHueHistogram_[x].resize(imageHeight_);
            
            for (int y = 0; y < imageHeight_; ++y) {
                cumulativeHueHistogram_[x][y].resize(HUE_BINS, 0);
                
                // Get current pixel
                const Utils::HSLAPixel* currentPixel = image.getPixel(x, y);
                
                // Convert hue to cartesian coordinates for circular averaging
                double hueRadians = currentPixel->hue * PI / 180.0;
                double currentHueX = currentPixel->saturation * std::cos(hueRadians);
                double currentHueY = currentPixel->saturation * std::sin(hueRadians);
                
                // Calculate cumulative values
                double cumulativeX = currentHueX;
                double cumulativeY = currentHueY;
                double cumulativeS = currentPixel->saturation;
                double cumulativeL = currentPixel->luminance;
                
                // Initialize histogram for current position
                std::vector<int> currentHistogram(HUE_BINS, 0);
                int hueBinIndex = static_cast<int>(currentPixel->hue / 10.0);
                hueBinIndex = std::min(hueBinIndex, HUE_BINS - 1); // Clamp to valid range
                currentHistogram[hueBinIndex] = 1;
                
                // Add contributions from neighboring cumulative regions
                if (x > 0 && y > 0) {
                    // Interior case: add top, left, subtract top-left
                    cumulativeX += cumulativeHueX_[x-1][y] + cumulativeHueX_[x][y-1] - cumulativeHueX_[x-1][y-1];
                    cumulativeY += cumulativeHueY_[x-1][y] + cumulativeHueY_[x][y-1] - cumulativeHueY_[x-1][y-1];
                    cumulativeS += cumulativeSaturation_[x-1][y] + cumulativeSaturation_[x][y-1] - cumulativeSaturation_[x-1][y-1];
                    cumulativeL += cumulativeLuminance_[x-1][y] + cumulativeLuminance_[x][y-1] - cumulativeLuminance_[x-1][y-1];
                    
                    currentHistogram = addHistograms(cumulativeHueHistogram_[x-1][y], cumulativeHueHistogram_[x][y-1]);
                    currentHistogram = subtractHistograms(currentHistogram, cumulativeHueHistogram_[x-1][y-1]);
                    currentHistogram[hueBinIndex]++;
                } else if (x > 0) {
                    // Left edge: add from left
                    cumulativeX += cumulativeHueX_[x-1][y];
                    cumulativeY += cumulativeHueY_[x-1][y];
                    cumulativeS += cumulativeSaturation_[x-1][y];
                    cumulativeL += cumulativeLuminance_[x-1][y];
                    
                    currentHistogram = cumulativeHueHistogram_[x-1][y];
                    currentHistogram[hueBinIndex]++;
                } else if (y > 0) {
                    // Top edge: add from above
                    cumulativeX += cumulativeHueX_[x][y-1];
                    cumulativeY += cumulativeHueY_[x][y-1];
                    cumulativeS += cumulativeSaturation_[x][y-1];
                    cumulativeL += cumulativeLuminance_[x][y-1];
                    
                    currentHistogram = cumulativeHueHistogram_[x][y-1];
                    currentHistogram[hueBinIndex]++;
                }
                // else: top-left corner, use initialized values
                
                // Store cumulative values
                cumulativeHueX_[x][y] = cumulativeX;
                cumulativeHueY_[x][y] = cumulativeY;
                cumulativeSaturation_[x][y] = cumulativeS;
                cumulativeLuminance_[x][y] = cumulativeL;
                cumulativeHueHistogram_[x][y] = currentHistogram;
            }
        }
    }

    Utils::HSLAPixel ImageStatistics::getAverageColor(const Rectangle& region) const {
        assert(isValidRectangle(region));
        
        double totalHueX, totalHueY, totalSaturation, totalLuminance;
        long pixelCount = getArea(region);
        
        int ulX = region.upperLeft.first;
        int ulY = region.upperLeft.second;
        int lrX = region.lowerRight.first;
        int lrY = region.lowerRight.second;
        
        if (ulX == 0 && ulY == 0) {
            // Region starts at origin
            totalHueX = cumulativeHueX_[lrX][lrY];
            totalHueY = cumulativeHueY_[lrX][lrY];
            totalSaturation = cumulativeSaturation_[lrX][lrY];
            totalLuminance = cumulativeLuminance_[lrX][lrY];
        } else if (ulX == 0) {
            // Region on left edge
            totalHueX = cumulativeHueX_[lrX][lrY] - cumulativeHueX_[lrX][ulY-1];
            totalHueY = cumulativeHueY_[lrX][lrY] - cumulativeHueY_[lrX][ulY-1];
            totalSaturation = cumulativeSaturation_[lrX][lrY] - cumulativeSaturation_[lrX][ulY-1];
            totalLuminance = cumulativeLuminance_[lrX][lrY] - cumulativeLuminance_[lrX][ulY-1];
        } else if (ulY == 0) {
            // Region on top edge
            totalHueX = cumulativeHueX_[lrX][lrY] - cumulativeHueX_[ulX-1][lrY];
            totalHueY = cumulativeHueY_[lrX][lrY] - cumulativeHueY_[ulX-1][lrY];
            totalSaturation = cumulativeSaturation_[lrX][lrY] - cumulativeSaturation_[ulX-1][lrY];
            totalLuminance = cumulativeLuminance_[lrX][lrY] - cumulativeLuminance_[ulX-1][lrY];
        } else {
            // Interior region
            totalHueX = cumulativeHueX_[lrX][lrY] - cumulativeHueX_[ulX-1][lrY] 
                       - cumulativeHueX_[lrX][ulY-1] + cumulativeHueX_[ulX-1][ulY-1];
            totalHueY = cumulativeHueY_[lrX][lrY] - cumulativeHueY_[ulX-1][lrY] 
                       - cumulativeHueY_[lrX][ulY-1] + cumulativeHueY_[ulX-1][ulY-1];
            totalSaturation = cumulativeSaturation_[lrX][lrY] - cumulativeSaturation_[ulX-1][lrY] 
                             - cumulativeSaturation_[lrX][ulY-1] + cumulativeSaturation_[ulX-1][ulY-1];
            totalLuminance = cumulativeLuminance_[lrX][lrY] - cumulativeLuminance_[ulX-1][lrY] 
                            - cumulativeLuminance_[lrX][ulY-1] + cumulativeLuminance_[ulX-1][ulY-1];
        }
        
        // Calculate averages
        double avgHueX = totalHueX / pixelCount;
        double avgHueY = totalHueY / pixelCount;
        double avgSaturation = totalSaturation / pixelCount;
        double avgLuminance = totalLuminance / pixelCount;
        
        // Convert back to hue angle
        double avgHue = std::atan2(avgHueY, avgHueX) * 180.0 / PI;
        if (avgHue < 0) avgHue += 360.0; // Ensure positive angle
        
        return Utils::HSLAPixel(avgHue, avgSaturation, avgLuminance, 1.0);
    }

    long ImageStatistics::getArea(const Rectangle& region) const {
        assert(isValidRectangle(region));
        
        int width = region.lowerRight.first - region.upperLeft.first + 1;
        int height = region.lowerRight.second - region.upperLeft.second + 1;
        return static_cast<long>(width) * height;
    }

    double ImageStatistics::calculateEntropy(const Rectangle& region) const {
        std::vector<int> histogram = buildHueHistogram(region);
        long area = getArea(region);
        return calculateEntropyFromDistribution(histogram, area);
    }

    std::vector<int> ImageStatistics::buildHueHistogram(const Rectangle& region) const {
        assert(isValidRectangle(region));
        
        int ulX = region.upperLeft.first;
        int ulY = region.upperLeft.second;
        int lrX = region.lowerRight.first;
        int lrY = region.lowerRight.second;
        
        std::vector<int> histogram(HUE_BINS, 0);
        
        if (ulX == 0 && ulY == 0) {
            histogram = cumulativeHueHistogram_[lrX][lrY];
        } else if (ulX == 0) {
            histogram = subtractHistograms(cumulativeHueHistogram_[lrX][lrY], 
                                         cumulativeHueHistogram_[lrX][ulY-1]);
        } else if (ulY == 0) {
            histogram = subtractHistograms(cumulativeHueHistogram_[lrX][lrY], 
                                         cumulativeHueHistogram_[ulX-1][lrY]);
        } else {
            histogram = cumulativeHueHistogram_[lrX][lrY];
            histogram = subtractHistograms(histogram, cumulativeHueHistogram_[ulX-1][lrY]);
            histogram = subtractHistograms(histogram, cumulativeHueHistogram_[lrX][ulY-1]);
            histogram = addHistograms(histogram, cumulativeHueHistogram_[ulX-1][ulY-1]);
        }
        
        return histogram;
    }

    std::vector<int> ImageStatistics::subtractHistograms(const std::vector<int>& first, 
                                                        const std::vector<int>& second) const {
        assert(first.size() == second.size());
        std::vector<int> result(first.size());
        
        for (size_t i = 0; i < first.size(); ++i) {
            result[i] = first[i] - second[i];
        }
        
        return result;
    }

    std::vector<int> ImageStatistics::addHistograms(const std::vector<int>& first, 
                                                   const std::vector<int>& second) const {
        assert(first.size() == second.size());
        std::vector<int> result(first.size());
        
        for (size_t i = 0; i < first.size(); ++i) {
            result[i] = first[i] + second[i];
        }
        
        return result;
    }

    double ImageStatistics::calculateEntropyFromDistribution(const std::vector<int>& distribution, 
                                                           int totalArea) const {
        if (totalArea <= 0) return 0.0;
        
        double entropy = 0.0;
        
        for (int count : distribution) {
            if (count > 0) {
                double probability = static_cast<double>(count) / totalArea;
                entropy -= probability * std::log2(probability);
            }
        }
        
        return entropy;
    }

    bool ImageStatistics::isValidRectangle(const Rectangle& region) const {
        return region.upperLeft.first >= 0 && region.upperLeft.second >= 0 &&
               region.lowerRight.first < imageWidth_ && region.lowerRight.second < imageHeight_ &&
               region.upperLeft.first <= region.lowerRight.first &&
               region.upperLeft.second <= region.lowerRight.second;
    }

} // namespace ImageCompression 