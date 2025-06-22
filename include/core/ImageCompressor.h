#ifndef IMAGE_COMPRESSION_IMAGE_COMPRESSOR_H
#define IMAGE_COMPRESSION_IMAGE_COMPRESSOR_H

#include "../utils/image/PNG.h"
#include "AdaptiveImageTree.h"
#include <string>
#include <vector>

namespace ImageCompression {

    // Old quality levels - still works but the 0.0-1.0 system is much better
    enum class CompressionQuality {
        HIGHEST_QUALITY,    // Barely any compression, looks great
        HIGH_QUALITY,       // Light compression
        MEDIUM_QUALITY,     // Good balance
        LOW_QUALITY,        // Pretty aggressive compression
        LOWEST_QUALITY      // Maximum compression, might look rough
    };

    // Everything you get back after compressing an image
    struct CompressionResult {
        Utils::PNG compressedImage;
        double compressionRatio;
        size_t originalPixels;
        size_t compressedRegions;
        double processingTimeSeconds;
        
        CompressionResult(const Utils::PNG& image, double ratio, 
                         size_t origPixels, size_t regions, double time)
            : compressedImage(image), compressionRatio(ratio), 
              originalPixels(origPixels), compressedRegions(regions),
              processingTimeSeconds(time) {}
    };

    // Main class for compressing images - this is what you'll use most of the time
    // It uses a smart tree algorithm that preserves important details while throwing away redundant stuff
    class ImageCompressor {
    public:
        // Compress an image with a quality from 0.0 to 1.0
        // 0.0 = tiny file, might look pixelated
        // 1.0 = huge file, looks perfect
        // 0.5 = usually a good starting point
        static CompressionResult compressImage(const Utils::PNG& inputImage,
                                             double qualityScore = 0.5);

        // Old way of setting quality - still works fine
        static CompressionResult compressImage(const Utils::PNG& inputImage,
                                             CompressionQuality quality);
        
        // If you want to mess with the internal settings (most people won't need this)
        static CompressionResult compressImage(const Utils::PNG& inputImage,
                                             const PruningConfig& config);
        
        // Load a PNG file, compress it, and save it - the easy way to compress files
        static CompressionResult compressImageFile(const std::string& inputFilePath,
                                                  const std::string& outputFilePath,
                                                  double qualityScore = 0.5);

        // Same thing but with the old quality system
        static CompressionResult compressImageFile(const std::string& inputFilePath,
                                                  const std::string& outputFilePath,
                                                  CompressionQuality quality);
        
        // Compress the same image at multiple quality levels for comparison
        static std::vector<CompressionResult> generateCompressionSeries(const Utils::PNG& inputImage,
                                                                       const std::string& outputPrefix);
        
        // Convert a quality number to the internal settings the algorithm uses
        static PruningConfig getConfigForQuality(double qualityScore);

        // Same thing but for the old quality system
        static PruningConfig getConfigForQuality(CompressionQuality quality);
        
        // Get a nice name for a quality score (like "high" or "medium")
        static std::string getQualityName(double qualityScore);

        // Get a nice name for the old quality levels
        static std::string getQualityName(CompressionQuality quality);
        
    private:
        // The actual compression work happens here - builds tree, prunes it, renders result
        static CompressionResult performCompression(const Utils::PNG& inputImage,
                                                  const PruningConfig& config);
    };

} // namespace ImageCompression

#endif // IMAGE_COMPRESSION_IMAGE_COMPRESSOR_H 