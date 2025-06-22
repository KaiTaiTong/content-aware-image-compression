#include "../../include/core/ImageCompressor.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace ImageCompression {

    CompressionResult ImageCompressor::compressImage(const Utils::PNG& inputImage,
                                                   double qualityScore) {
        PruningConfig config = getConfigForQuality(qualityScore);
        return performCompression(inputImage, config);
    }

    CompressionResult ImageCompressor::compressImage(const Utils::PNG& inputImage,
                                                   CompressionQuality quality) {
        PruningConfig config = getConfigForQuality(quality);
        return performCompression(inputImage, config);
    }

    CompressionResult ImageCompressor::compressImage(const Utils::PNG& inputImage,
                                                   const PruningConfig& config) {
        return performCompression(inputImage, config);
    }

    CompressionResult ImageCompressor::compressImageFile(const std::string& inputFilePath,
                                                       const std::string& outputFilePath,
                                                       double qualityScore) {
        // Load input image
        Utils::PNG inputImage;
        if (!inputImage.loadFromFile(inputFilePath)) {
            throw std::runtime_error("Failed to load image from: " + inputFilePath);
        }
        
        // Perform compression
        CompressionResult result = compressImage(inputImage, qualityScore);
        
        // Save compressed image
        if (!result.compressedImage.saveToFile(outputFilePath)) {
            throw std::runtime_error("Failed to save compressed image to: " + outputFilePath);
        }
        
        return result;
    }

    CompressionResult ImageCompressor::compressImageFile(const std::string& inputFilePath,
                                                       const std::string& outputFilePath,
                                                       CompressionQuality quality) {
        // Load input image
        Utils::PNG inputImage;
        if (!inputImage.loadFromFile(inputFilePath)) {
            throw std::runtime_error("Failed to load image from: " + inputFilePath);
        }
        
        // Perform compression
        CompressionResult result = compressImage(inputImage, quality);
        
        // Save compressed image
        if (!result.compressedImage.saveToFile(outputFilePath)) {
            throw std::runtime_error("Failed to save compressed image to: " + outputFilePath);
        }
        
        return result;
    }

    std::vector<CompressionResult> ImageCompressor::generateCompressionSeries(
        const Utils::PNG& inputImage, const std::string& outputPrefix) {
        
        std::vector<CompressionResult> results;
        std::vector<CompressionQuality> qualities = {
            CompressionQuality::HIGHEST_QUALITY,
            CompressionQuality::HIGH_QUALITY,
            CompressionQuality::MEDIUM_QUALITY,
            CompressionQuality::LOW_QUALITY,
            CompressionQuality::LOWEST_QUALITY
        };
        
        for (CompressionQuality quality : qualities) {
            CompressionResult result = compressImage(inputImage, quality);
            
            // Save the compressed image
            std::string filename = outputPrefix + "-" + getQualityName(quality) + ".png";
            result.compressedImage.saveToFile(filename);
            
            results.push_back(std::move(result));
        }
        
        return results;
    }

    PruningConfig ImageCompressor::getConfigForQuality(double qualityScore) {
        // Clamp quality score to valid range [0.0, 1.0]
        qualityScore = std::max(0.0, std::min(1.0, qualityScore));
        
        // We use exponential curves so small quality changes make a big difference
        // This way 0.50 vs 0.51 actually produces visibly different results
        
        // Higher quality = pickier about what regions to merge
        // 0.0 quality = merge anything that's vaguely similar (0.85 threshold)
        // 1.0 quality = only merge nearly identical regions (0.995 threshold)
        double similarity = 0.85 + 0.145 * std::pow(qualityScore, 1.5);
        
        // Higher quality = colors need to be much closer to count as "similar"
        // 0.0 quality = pretty loose color matching (0.30 tolerance)
        // 1.0 quality = very strict color matching (0.005 tolerance)
        double tolerance = 0.30 * std::pow(1.0 - qualityScore, 2.0);
        
        // Don't let tolerance get too small or weird stuff happens
        tolerance = std::max(tolerance, 0.005);
        
        return PruningConfig(similarity, tolerance);
    }

    PruningConfig ImageCompressor::getConfigForQuality(CompressionQuality quality) {
        switch (quality) {
            case CompressionQuality::HIGHEST_QUALITY:
                return PruningConfig(0.99, 0.025);  // Super picky, tiny files
            case CompressionQuality::HIGH_QUALITY:
                return PruningConfig(0.99, 0.05);   // Pretty picky, small files
            case CompressionQuality::MEDIUM_QUALITY:
                return PruningConfig(0.99, 0.1);    // Balanced approach
            case CompressionQuality::LOW_QUALITY:
                return PruningConfig(0.95, 0.15);   // More aggressive compression
            case CompressionQuality::LOWEST_QUALITY:
                return PruningConfig(0.90, 0.2);    // Go nuts with compression
            default:
                return PruningConfig(0.99, 0.1);    // When in doubt, use medium
        }
    }

    std::string ImageCompressor::getQualityName(double qualityScore) {
        // Clamp quality score to valid range [0.0, 1.0]
        qualityScore = std::max(0.0, std::min(1.0, qualityScore));
        
        if (qualityScore >= 0.9) return "highest";
        if (qualityScore >= 0.7) return "high";
        if (qualityScore >= 0.3) return "medium";
        if (qualityScore >= 0.1) return "low";
        return "lowest";
    }

    std::string ImageCompressor::getQualityName(CompressionQuality quality) {
        switch (quality) {
            case CompressionQuality::HIGHEST_QUALITY:
                return "highest-quality";
            case CompressionQuality::HIGH_QUALITY:
                return "high-quality";
            case CompressionQuality::MEDIUM_QUALITY:
                return "medium-quality";
            case CompressionQuality::LOW_QUALITY:
                return "low-quality";
            case CompressionQuality::LOWEST_QUALITY:
                return "lowest-quality";
            default:
                return "unknown-quality";
        }
    }

    CompressionResult ImageCompressor::performCompression(const Utils::PNG& inputImage,
                                                        const PruningConfig& config) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Build the adaptive tree
        AdaptiveImageTree tree(inputImage);
        
        // Store original statistics
        size_t originalPixels = static_cast<size_t>(inputImage.getWidth()) * inputImage.getHeight();
        
        // Prune the tree based on configuration
        tree.pruneTree(config);
        
        // Render the compressed image
        Utils::PNG compressedImage = tree.renderToImage();
        
        // Calculate final statistics
        size_t compressedRegions = tree.countLeafNodes();
        double compressionRatio = tree.getCompressionRatio();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        double processingTime = duration.count() / 1000.0; // Convert to seconds
        
        return CompressionResult(compressedImage, compressionRatio, originalPixels,
                               compressedRegions, processingTime);
    }

} // namespace ImageCompression 