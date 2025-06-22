#ifndef IMAGE_COMPRESSION_ADAPTIVE_IMAGE_TREE_H
#define IMAGE_COMPRESSION_ADAPTIVE_IMAGE_TREE_H

#include "../utils/image/PNG.h"
#include "../utils/image/HSLAPixel.h"
#include "../statistics/ImageStatistics.h"
#include <memory>
#include <utility>

namespace ImageCompression {

    // Settings that control how aggressively we compress the image
    struct PruningConfig {
        double minimumSimilarityPercentage;  // How similar colors need to be to merge regions
        double colorToleranceThreshold;      // How close colors need to be to count as "similar"
        
        PruningConfig(double minSimilarity = 0.95, double tolerance = 0.1)
            : minimumSimilarityPercentage(minSimilarity)
            , colorToleranceThreshold(tolerance) {}
    };

    // The heart of the compression algorithm - a tree that splits the image into regions
    // Complex areas get more detail, simple areas get merged together
    // It's like a smart version of those old-school pixel art converters
    class AdaptiveImageTree {
    private:
        // Each node represents a rectangular chunk of the image
        struct TreeNode {
            Rectangle region;                       // What part of the image this covers
            Utils::HSLAPixel averageColor;         // The average color for this region
            std::unique_ptr<TreeNode> leftChild;   // Left or top half when we split
            std::unique_ptr<TreeNode> rightChild;  // Right or bottom half when we split
            
            TreeNode(const Rectangle& rect, const Utils::HSLAPixel& avgColor)
                : region(rect), averageColor(avgColor), leftChild(nullptr), rightChild(nullptr) {}
        };
        
    public:
        // Build the tree from an image - this analyzes the whole thing and creates the structure
        explicit AdaptiveImageTree(const Utils::PNG& inputImage);
        
        // Copy constructor - make a duplicate tree
        AdaptiveImageTree(const AdaptiveImageTree& other);
        
        // Assignment - copy one tree to another
        AdaptiveImageTree& operator=(const AdaptiveImageTree& rhs);
        
        // Destructor - smart pointers clean up automatically
        ~AdaptiveImageTree() = default;
        
        // Turn the tree back into a PNG image - this is where you see the compression results
        Utils::PNG renderToImage() const;
        
        // Remove unnecessary detail from the tree based on how similar colors are
        void pruneTree(const PruningConfig& config);
        
        // Get the original image size
        std::pair<int, int> getImageDimensions() const;
        
        // Count how many regions we ended up with (fewer = more compression)
        size_t countLeafNodes() const;
        
        // Figure out how much we compressed it (smaller number = more compression)
        double getCompressionRatio() const;
        
    private:
        std::unique_ptr<TreeNode> rootNode_;
        int imageWidth_;
        int imageHeight_;
        
        // Build the tree by recursively splitting regions where it makes sense
        std::unique_ptr<TreeNode> buildTreeRecursive(const ImageStatistics& statistics,
                                                    const Rectangle& region);
        
        // Find the best place to split a region (tries horizontal and vertical splits)
        std::pair<Rectangle, Rectangle> findOptimalSplit(const ImageStatistics& statistics,
                                                        const Rectangle& region);
        
        // Walk through the tree and fill in pixels in the output image
        void renderNodeRecursive(Utils::PNG& outputImage, 
                                const TreeNode* node) const;
        
        // Make a deep copy of a tree branch
        std::unique_ptr<TreeNode> copyTreeRecursive(const TreeNode* sourceNode);
        
        // Walk through the tree and remove branches that don't add much detail
        void pruneNodeRecursive(std::unique_ptr<TreeNode>& node, 
                               const PruningConfig& config);
        
        // Check if a tree branch is simple enough that we can just use one color for the whole thing
        bool shouldPruneSubtree(const TreeNode* node, const PruningConfig& config) const;
        
        // Count how many pixels in a tree branch are similar to a reference color
        int countSimilarPixels(const TreeNode* node, 
                             const Utils::HSLAPixel& referenceColor,
                             double tolerance, 
                             int& totalPixels) const;
        
        // Count leaf nodes in a tree branch
        size_t countLeafNodesRecursive(const TreeNode* node) const;
        
        // Figure out how different two colors are (in a way that matches human vision)
        double calculateColorDistance(const Utils::HSLAPixel& color1,
                                    const Utils::HSLAPixel& color2) const;
    };

} // namespace ImageCompression

#endif // IMAGE_COMPRESSION_ADAPTIVE_IMAGE_TREE_H 