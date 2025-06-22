#include "../../include/statistics/ImageStatistics.h"
#include <cmath>
#include <algorithm>
#include <cassert>

namespace ImageCompression {

    // Static member definitions
    std::vector<double> ImageStatistics::cosLookup_;
    std::vector<double> ImageStatistics::sinLookup_;
    bool ImageStatistics::lookupTablesInitialized_ = false;

    void ImageStatistics::initializeLookupTables() {
        if (lookupTablesInitialized_) return;
        
        cosLookup_.resize(360);
        sinLookup_.resize(360);
        
        for (int i = 0; i < 360; ++i) {
            double radians = i * PI / 180.0;
            cosLookup_[i] = std::cos(radians);
            sinLookup_[i] = std::sin(radians);
        }
        
        lookupTablesInitialized_ = true;
    }

    ImageStatistics::ImageStatistics(const Utils::PNG& image) 
        : imageWidth_(image.getWidth()), imageHeight_(image.getHeight()) {
        
        // Initialize lookup tables once
        initializeLookupTables();
        
        // Pre-allocate flat arrays
        size_t totalPixels = static_cast<size_t>(imageWidth_) * imageHeight_;
        cumulativeHueX_.resize(totalPixels);
        cumulativeHueY_.resize(totalPixels);
        cumulativeSaturation_.resize(totalPixels);
        cumulativeLuminance_.resize(totalPixels);
        cumulativeHueHistogram_.resize(totalPixels * HUE_BINS, 0);
        
        // Build cumulative arrays using flat indexing
        for (int y = 0; y < imageHeight_; ++y) {
            for (int x = 0; x < imageWidth_; ++x) {
                size_t currentIndex = getIndex(x, y);
                
                // Get current pixel
                const Utils::HSLAPixel* currentPixel = image.getPixel(x, y);
                
                // Convert hue to cartesian coordinates using fast lookup
                double currentHueX = currentPixel->saturation * fastCos(currentPixel->hue);
                double currentHueY = currentPixel->saturation * fastSin(currentPixel->hue);
                
                // Calculate cumulative values
                double cumulativeX = currentHueX;
                double cumulativeY = currentHueY;
                double cumulativeS = currentPixel->saturation;
                double cumulativeL = currentPixel->luminance;
                
                // Initialize histogram for current position
                int hueBinIndex = static_cast<int>(currentPixel->hue / 10.0);
                hueBinIndex = std::min(hueBinIndex, HUE_BINS - 1);
                
                // Add contributions from neighboring cumulative regions
                if (x > 0 && y > 0) {
                    // Interior case: add top, left, subtract top-left
                    size_t leftIndex = getIndex(x-1, y);
                    size_t topIndex = getIndex(x, y-1);
                    size_t topLeftIndex = getIndex(x-1, y-1);
                    
                    cumulativeX += cumulativeHueX_[leftIndex] + cumulativeHueX_[topIndex] - cumulativeHueX_[topLeftIndex];
                    cumulativeY += cumulativeHueY_[leftIndex] + cumulativeHueY_[topIndex] - cumulativeHueY_[topLeftIndex];
                    cumulativeS += cumulativeSaturation_[leftIndex] + cumulativeSaturation_[topIndex] - cumulativeSaturation_[topLeftIndex];
                    cumulativeL += cumulativeLuminance_[leftIndex] + cumulativeLuminance_[topIndex] - cumulativeLuminance_[topLeftIndex];
                    
                    // Update histogram in-place (no vector copying!)
                    for (int bin = 0; bin < HUE_BINS; ++bin) {
                        size_t histIndex = getHistogramIndex(x, y, bin);
                        size_t leftHistIndex = getHistogramIndex(x-1, y, bin);
                        size_t topHistIndex = getHistogramIndex(x, y-1, bin);
                        size_t topLeftHistIndex = getHistogramIndex(x-1, y-1, bin);
                        
                        cumulativeHueHistogram_[histIndex] = cumulativeHueHistogram_[leftHistIndex] 
                                                           + cumulativeHueHistogram_[topHistIndex] 
                                                           - cumulativeHueHistogram_[topLeftHistIndex];
                    }
                    cumulativeHueHistogram_[getHistogramIndex(x, y, hueBinIndex)]++;
                    
                } else if (x > 0) {
                    // Left edge: add from left
                    size_t leftIndex = getIndex(x-1, y);
                    cumulativeX += cumulativeHueX_[leftIndex];
                    cumulativeY += cumulativeHueY_[leftIndex];
                    cumulativeS += cumulativeSaturation_[leftIndex];
                    cumulativeL += cumulativeLuminance_[leftIndex];
                    
                    for (int bin = 0; bin < HUE_BINS; ++bin) {
                        cumulativeHueHistogram_[getHistogramIndex(x, y, bin)] = 
                            cumulativeHueHistogram_[getHistogramIndex(x-1, y, bin)];
                    }
                    cumulativeHueHistogram_[getHistogramIndex(x, y, hueBinIndex)]++;
                    
                } else if (y > 0) {
                    // Top edge: add from above
                    size_t topIndex = getIndex(x, y-1);
                    cumulativeX += cumulativeHueX_[topIndex];
                    cumulativeY += cumulativeHueY_[topIndex];
                    cumulativeS += cumulativeSaturation_[topIndex];
                    cumulativeL += cumulativeLuminance_[topIndex];
                    
                    for (int bin = 0; bin < HUE_BINS; ++bin) {
                        cumulativeHueHistogram_[getHistogramIndex(x, y, bin)] = 
                            cumulativeHueHistogram_[getHistogramIndex(x, y-1, bin)];
                    }
                    cumulativeHueHistogram_[getHistogramIndex(x, y, hueBinIndex)]++;
                } else {
                    // Top-left corner: just set the current pixel's histogram
                    cumulativeHueHistogram_[getHistogramIndex(x, y, hueBinIndex)] = 1;
                }
                
                // Store cumulative values
                cumulativeHueX_[currentIndex] = cumulativeX;
                cumulativeHueY_[currentIndex] = cumulativeY;
                cumulativeSaturation_[currentIndex] = cumulativeS;
                cumulativeLuminance_[currentIndex] = cumulativeL;
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
            size_t lrIndex = getIndex(lrX, lrY);
            totalHueX = cumulativeHueX_[lrIndex];
            totalHueY = cumulativeHueY_[lrIndex];
            totalSaturation = cumulativeSaturation_[lrIndex];
            totalLuminance = cumulativeLuminance_[lrIndex];
        } else if (ulX == 0) {
            // Region on left edge
            size_t lrIndex = getIndex(lrX, lrY);
            size_t topIndex = getIndex(lrX, ulY-1);
            totalHueX = cumulativeHueX_[lrIndex] - cumulativeHueX_[topIndex];
            totalHueY = cumulativeHueY_[lrIndex] - cumulativeHueY_[topIndex];
            totalSaturation = cumulativeSaturation_[lrIndex] - cumulativeSaturation_[topIndex];
            totalLuminance = cumulativeLuminance_[lrIndex] - cumulativeLuminance_[topIndex];
        } else if (ulY == 0) {
            // Region on top edge
            size_t lrIndex = getIndex(lrX, lrY);
            size_t leftIndex = getIndex(ulX-1, lrY);
            totalHueX = cumulativeHueX_[lrIndex] - cumulativeHueX_[leftIndex];
            totalHueY = cumulativeHueY_[lrIndex] - cumulativeHueY_[leftIndex];
            totalSaturation = cumulativeSaturation_[lrIndex] - cumulativeSaturation_[leftIndex];
            totalLuminance = cumulativeLuminance_[lrIndex] - cumulativeLuminance_[leftIndex];
        } else {
            // Interior region
            size_t lrIndex = getIndex(lrX, lrY);
            size_t leftIndex = getIndex(ulX-1, lrY);
            size_t topIndex = getIndex(lrX, ulY-1);
            size_t topLeftIndex = getIndex(ulX-1, ulY-1);
            
            totalHueX = cumulativeHueX_[lrIndex] - cumulativeHueX_[leftIndex] 
                       - cumulativeHueX_[topIndex] + cumulativeHueX_[topLeftIndex];
            totalHueY = cumulativeHueY_[lrIndex] - cumulativeHueY_[leftIndex] 
                       - cumulativeHueY_[topIndex] + cumulativeHueY_[topLeftIndex];
            totalSaturation = cumulativeSaturation_[lrIndex] - cumulativeSaturation_[leftIndex] 
                             - cumulativeSaturation_[topIndex] + cumulativeSaturation_[topLeftIndex];
            totalLuminance = cumulativeLuminance_[lrIndex] - cumulativeLuminance_[leftIndex] 
                            - cumulativeLuminance_[topIndex] + cumulativeLuminance_[topLeftIndex];
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

    double ImageStatistics::calculateEntropyOptimized(const Rectangle& region, std::vector<int>& histogramBuffer) const {
        buildHueHistogramOptimized(region, histogramBuffer);
        long area = getArea(region);
        return calculateEntropyFromDistribution(histogramBuffer, area);
    }

    std::vector<int> ImageStatistics::buildHueHistogram(const Rectangle& region) const {
        assert(isValidRectangle(region));
        
        int ulX = region.upperLeft.first;
        int ulY = region.upperLeft.second;
        int lrX = region.lowerRight.first;
        int lrY = region.lowerRight.second;
        
        std::vector<int> histogram(HUE_BINS, 0);
        
        if (ulX == 0 && ulY == 0) {
            // Region starts at origin
            for (int bin = 0; bin < HUE_BINS; ++bin) {
                histogram[bin] = cumulativeHueHistogram_[getHistogramIndex(lrX, lrY, bin)];
            }
        } else if (ulX == 0) {
            // Region on left edge
            for (int bin = 0; bin < HUE_BINS; ++bin) {
                histogram[bin] = cumulativeHueHistogram_[getHistogramIndex(lrX, lrY, bin)] 
                               - cumulativeHueHistogram_[getHistogramIndex(lrX, ulY-1, bin)];
            }
        } else if (ulY == 0) {
            // Region on top edge
            for (int bin = 0; bin < HUE_BINS; ++bin) {
                histogram[bin] = cumulativeHueHistogram_[getHistogramIndex(lrX, lrY, bin)] 
                               - cumulativeHueHistogram_[getHistogramIndex(ulX-1, lrY, bin)];
            }
        } else {
            // Interior region
            for (int bin = 0; bin < HUE_BINS; ++bin) {
                histogram[bin] = cumulativeHueHistogram_[getHistogramIndex(lrX, lrY, bin)]
                               - cumulativeHueHistogram_[getHistogramIndex(ulX-1, lrY, bin)]
                               - cumulativeHueHistogram_[getHistogramIndex(lrX, ulY-1, bin)]
                               + cumulativeHueHistogram_[getHistogramIndex(ulX-1, ulY-1, bin)];
            }
        }
        
        return histogram;
    }

    void ImageStatistics::buildHueHistogramOptimized(const Rectangle& region, std::vector<int>& histogramBuffer) const {
        assert(isValidRectangle(region));
        
        // Ensure buffer is the right size and clear it
        if (histogramBuffer.size() != HUE_BINS) {
            histogramBuffer.resize(HUE_BINS);
        }
        std::fill(histogramBuffer.begin(), histogramBuffer.end(), 0);
        
        int ulX = region.upperLeft.first;
        int ulY = region.upperLeft.second;
        int lrX = region.lowerRight.first;
        int lrY = region.lowerRight.second;
        
        if (ulX == 0 && ulY == 0) {
            // Region starts at origin
            for (int bin = 0; bin < HUE_BINS; ++bin) {
                histogramBuffer[bin] = cumulativeHueHistogram_[getHistogramIndex(lrX, lrY, bin)];
            }
        } else if (ulX == 0) {
            // Region on left edge
            for (int bin = 0; bin < HUE_BINS; ++bin) {
                histogramBuffer[bin] = cumulativeHueHistogram_[getHistogramIndex(lrX, lrY, bin)] 
                                     - cumulativeHueHistogram_[getHistogramIndex(lrX, ulY-1, bin)];
            }
        } else if (ulY == 0) {
            // Region on top edge
            for (int bin = 0; bin < HUE_BINS; ++bin) {
                histogramBuffer[bin] = cumulativeHueHistogram_[getHistogramIndex(lrX, lrY, bin)] 
                                     - cumulativeHueHistogram_[getHistogramIndex(ulX-1, lrY, bin)];
            }
        } else {
            // Interior region
            for (int bin = 0; bin < HUE_BINS; ++bin) {
                histogramBuffer[bin] = cumulativeHueHistogram_[getHistogramIndex(lrX, lrY, bin)]
                                     - cumulativeHueHistogram_[getHistogramIndex(ulX-1, lrY, bin)]
                                     - cumulativeHueHistogram_[getHistogramIndex(lrX, ulY-1, bin)]
                                     + cumulativeHueHistogram_[getHistogramIndex(ulX-1, ulY-1, bin)];
            }
        }
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