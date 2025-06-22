#include "../include/core/ImageCompressor.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>

using namespace ImageCompression;

void printUsage(const std::string& programName) {
    std::cout << "Content-Aware Image Compression Tool\n";
    std::cout << "====================================\n\n";
    std::cout << "Usage: " << programName << " <input_dir> <output_dir> [quality]\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  input_dir   - Directory containing input PNG images\n";
    std::cout << "  output_dir  - Directory where compressed images will be saved\n";
    std::cout << "  quality     - Compression quality (optional, default: 0.5)\n\n";
    std::cout << "Quality options:\n";
    std::cout << "  0.0 - 1.0   - Continuous quality scale (0.0 = maximum compression, 1.0 = minimal compression)\n";
    std::cout << "  highest     - Best quality, minimal compression (equivalent to 1.0)\n";
    std::cout << "  high        - High quality, light compression (equivalent to 0.8)\n";
    std::cout << "  medium      - Balanced quality and compression (equivalent to 0.5)\n";
    std::cout << "  low         - Lower quality, aggressive compression (equivalent to 0.2)\n";
    std::cout << "  lowest      - Minimum quality, maximum compression (equivalent to 0.0)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " ./input ./output\n";
    std::cout << "  " << programName << " ./photos ./compressed 0.75\n";
    std::cout << "  " << programName << " ./photos ./compressed high\n";
}

struct QualityValue {
    bool isFloat;
    double floatValue;
    CompressionQuality enumValue;
};

QualityValue parseQuality(const std::string& qualityStr) {
    // Try to parse as float first
    try {
        double value = std::stod(qualityStr);
        if (value >= 0.0 && value <= 1.0) {
            return {true, value, CompressionQuality::MEDIUM_QUALITY};
        } else {
            std::cerr << "Warning: Quality value " << value << " out of range [0.0, 1.0], using 0.5\n";
            return {true, 0.5, CompressionQuality::MEDIUM_QUALITY};
        }
    } catch (const std::exception&) {
        // Not a valid float, try string values
        if (qualityStr == "highest") return {false, 1.0, CompressionQuality::HIGHEST_QUALITY};
        if (qualityStr == "high") return {false, 0.8, CompressionQuality::HIGH_QUALITY};
        if (qualityStr == "medium") return {false, 0.5, CompressionQuality::MEDIUM_QUALITY};
        if (qualityStr == "low") return {false, 0.2, CompressionQuality::LOW_QUALITY};
        if (qualityStr == "lowest") return {false, 0.0, CompressionQuality::LOWEST_QUALITY};
        
        std::cerr << "Warning: Unknown quality '" << qualityStr << "', using 0.5\n";
        return {true, 0.5, CompressionQuality::MEDIUM_QUALITY};
    }
}

std::vector<std::string> findPngFiles(const std::string& directory) {
    std::vector<std::string> pngFiles;
    
    if (!std::filesystem::exists(directory)) {
        throw std::runtime_error("Input directory does not exist: " + directory);
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            std::string extension = entry.path().extension().string();
            
            // Convert extension to lowercase for comparison
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            
            if (extension == ".png") {
                pngFiles.push_back(entry.path().string());
            }
        }
    }
    
    return pngFiles;
}

void createOutputDirectory(const std::string& outputDir) {
    if (!std::filesystem::exists(outputDir)) {
        std::filesystem::create_directories(outputDir);
        std::cout << "Created output directory: " << outputDir << "\n";
    }
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        if (argc < 3 || argc > 4) {
            printUsage(argv[0]);
            return 1;
        }
        
        std::string inputDir = argv[1];
        std::string outputDir = argv[2];
        QualityValue qualityValue = {true, 0.5, CompressionQuality::MEDIUM_QUALITY}; // Default to 0.5
        
        if (argc == 4) {
            qualityValue = parseQuality(argv[3]);
        }
        
        // Create output directory if it doesn't exist
        createOutputDirectory(outputDir);
        
        // Find all PNG files in input directory
        std::vector<std::string> pngFiles = findPngFiles(inputDir);
        
        if (pngFiles.empty()) {
            std::cout << "No PNG files found in input directory: " << inputDir << "\n";
            return 0;
        }
        
        std::cout << "Found " << pngFiles.size() << " PNG file(s) to compress\n";
        if (qualityValue.isFloat) {
            std::cout << "Quality: " << std::fixed << std::setprecision(2) << qualityValue.floatValue 
                     << " (" << ImageCompressor::getQualityName(qualityValue.floatValue) << ")\n";
        } else {
            std::cout << "Quality: " << ImageCompressor::getQualityName(qualityValue.enumValue) << "\n";
        }
        std::cout << "Output directory: " << outputDir << "\n\n";
        
        // Process each image
        size_t processed = 0;
        double totalTime = 0.0;
        size_t totalOriginalPixels = 0;
        size_t totalCompressedRegions = 0;
        
        for (const std::string& inputPath : pngFiles) {
            std::filesystem::path inputFile(inputPath);
            std::string filename = inputFile.filename().string();
            
            // Create output filename with quality suffix
            std::string baseName = inputFile.stem().string();
            std::string qualitySuffix;
            if (qualityValue.isFloat) {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << qualityValue.floatValue;
                qualitySuffix = oss.str();
            } else {
                qualitySuffix = ImageCompressor::getQualityName(qualityValue.enumValue);
            }
            std::string outputFilename = baseName + "_q" + qualitySuffix + ".png";
            std::string outputPath = std::filesystem::path(outputDir) / outputFilename;
            
            std::cout << "Processing: " << filename << " -> " << outputFilename << " ... ";
            std::cout.flush();
            
            try {
                CompressionResult result = qualityValue.isFloat 
                    ? ImageCompressor::compressImageFile(inputPath, outputPath, qualityValue.floatValue)
                    : ImageCompressor::compressImageFile(inputPath, outputPath, qualityValue.enumValue);
                
                processed++;
                totalTime += result.processingTimeSeconds;
                totalOriginalPixels += result.originalPixels;
                totalCompressedRegions += result.compressedRegions;
                
                std::cout << "✓ (" << std::fixed << std::setprecision(1) 
                         << (result.compressionRatio * 100) << "% compression, "
                         << std::setprecision(2) << result.processingTimeSeconds << "s)\n";
                
            } catch (const std::exception& e) {
                std::cout << "✗ Error: " << e.what() << "\n";
            }
        }
        
        // Print summary
        std::cout << "\n=== Compression Summary ===\n";
        std::cout << "Files processed: " << processed << "/" << pngFiles.size() << "\n";
        std::cout << "Total processing time: " << std::fixed << std::setprecision(2) << totalTime << " seconds\n";
        
        if (processed > 0) {
            double avgCompressionRatio = static_cast<double>(totalCompressedRegions) / totalOriginalPixels;
            std::cout << "Average compression ratio: " << std::fixed << std::setprecision(1) 
                     << (avgCompressionRatio * 100) << "%\n";
            std::cout << "Average time per image: " << std::setprecision(2) 
                     << (totalTime / processed) << " seconds\n";
        }
        
        std::cout << "\nCompression complete! Check output directory: " << outputDir << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 