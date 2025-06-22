#include "../../include/core/AdaptiveImageTree.h"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>

namespace ImageCompression {

    AdaptiveImageTree::AdaptiveImageTree(const Utils::PNG& inputImage) 
        : imageWidth_(inputImage.getWidth()), imageHeight_(inputImage.getHeight()) {
        
        // Build statistics for the entire image
        ImageStatistics statistics(inputImage);
        
        // Create the root rectangle covering the entire image
        Rectangle rootRegion(0, 0, imageWidth_ - 1, imageHeight_ - 1);
        
        // Recursively build the tree
        rootNode_ = buildTreeRecursive(statistics, rootRegion);
    }

    AdaptiveImageTree::AdaptiveImageTree(const AdaptiveImageTree& other) 
        : imageWidth_(other.imageWidth_), imageHeight_(other.imageHeight_) {
        rootNode_ = copyTreeRecursive(other.rootNode_.get());
    }

    AdaptiveImageTree& AdaptiveImageTree::operator=(const AdaptiveImageTree& rhs) {
        if (this != &rhs) {
            imageWidth_ = rhs.imageWidth_;
            imageHeight_ = rhs.imageHeight_;
            rootNode_ = copyTreeRecursive(rhs.rootNode_.get());
        }
        return *this;
    }

    std::unique_ptr<AdaptiveImageTree::TreeNode> 
    AdaptiveImageTree::buildTreeRecursive(const ImageStatistics& statistics, 
                                         const Rectangle& region) {
        
        // Get average color for this region
        Utils::HSLAPixel averageColor = statistics.getAverageColor(region);
        
        // Create node for this region
        auto currentNode = std::make_unique<TreeNode>(region, averageColor);
        
        // Base case: single pixel region
        if (region.upperLeft == region.lowerRight) {
            return currentNode;
        }
        
        // Find optimal split for this region
        auto splitResult = findOptimalSplit(statistics, region);
        Rectangle leftRegion = splitResult.first;
        Rectangle rightRegion = splitResult.second;
        
        // Recursively build left and right subtrees
        currentNode->leftChild = buildTreeRecursive(statistics, leftRegion);
        currentNode->rightChild = buildTreeRecursive(statistics, rightRegion);
        
        return currentNode;
    }

    std::pair<Rectangle, Rectangle> 
    AdaptiveImageTree::findOptimalSplit(const ImageStatistics& statistics, 
                                       const Rectangle& region) {
        
        double bestWeightedEntropy = std::numeric_limits<double>::max();
        Rectangle bestLeftRegion(0, 0, 0, 0);
        Rectangle bestRightRegion(0, 0, 0, 0);
        
        int regionWidth = region.lowerRight.first - region.upperLeft.first + 1;
        int regionHeight = region.lowerRight.second - region.upperLeft.second + 1;
        long totalArea = statistics.getArea(region);
        
        // Try horizontal splits
        if (regionHeight > 1) {
            for (int splitY = region.upperLeft.second; splitY < region.lowerRight.second; ++splitY) {
                Rectangle topRegion(region.upperLeft.first, region.upperLeft.second,
                                   region.lowerRight.first, splitY);
                Rectangle bottomRegion(region.upperLeft.first, splitY + 1,
                                      region.lowerRight.first, region.lowerRight.second);
                
                double topEntropy = statistics.calculateEntropy(topRegion);
                double bottomEntropy = statistics.calculateEntropy(bottomRegion);
                long topArea = statistics.getArea(topRegion);
                long bottomArea = statistics.getArea(bottomRegion);
                
                double weightedEntropy = (topEntropy * topArea + bottomEntropy * bottomArea) / totalArea;
                
                if (weightedEntropy < bestWeightedEntropy) {
                    bestWeightedEntropy = weightedEntropy;
                    bestLeftRegion = topRegion;
                    bestRightRegion = bottomRegion;
                }
            }
        }
        
        // Try vertical splits
        if (regionWidth > 1) {
            for (int splitX = region.upperLeft.first; splitX < region.lowerRight.first; ++splitX) {
                Rectangle leftRegion(region.upperLeft.first, region.upperLeft.second,
                                    splitX, region.lowerRight.second);
                Rectangle rightRegion(splitX + 1, region.upperLeft.second,
                                     region.lowerRight.first, region.lowerRight.second);
                
                double leftEntropy = statistics.calculateEntropy(leftRegion);
                double rightEntropy = statistics.calculateEntropy(rightRegion);
                long leftArea = statistics.getArea(leftRegion);
                long rightArea = statistics.getArea(rightRegion);
                
                double weightedEntropy = (leftEntropy * leftArea + rightEntropy * rightArea) / totalArea;
                
                if (weightedEntropy < bestWeightedEntropy) {
                    bestWeightedEntropy = weightedEntropy;
                    bestLeftRegion = leftRegion;
                    bestRightRegion = rightRegion;
                }
            }
        }
        
        return std::make_pair(bestLeftRegion, bestRightRegion);
    }

    Utils::PNG AdaptiveImageTree::renderToImage() const {
        Utils::PNG outputImage(imageWidth_, imageHeight_);
        
        if (rootNode_) {
            renderNodeRecursive(outputImage, rootNode_.get());
        }
        
        return outputImage;
    }

    void AdaptiveImageTree::renderNodeRecursive(Utils::PNG& outputImage, 
                                               const TreeNode* node) const {
        if (!node) return;
        
        // If this region didn't get split further, just fill it with one color
        if (!node->leftChild && !node->rightChild) {
            for (int x = node->region.upperLeft.first; x <= node->region.lowerRight.first; ++x) {
                for (int y = node->region.upperLeft.second; y <= node->region.lowerRight.second; ++y) {
                    Utils::HSLAPixel* pixel = outputImage.getPixel(x, y);
                    *pixel = node->averageColor;
                }
            }
        } else {
            // This region got split, so render both halves
            if (node->leftChild) {
                renderNodeRecursive(outputImage, node->leftChild.get());
            }
            if (node->rightChild) {
                renderNodeRecursive(outputImage, node->rightChild.get());
            }
        }
    }

    std::unique_ptr<AdaptiveImageTree::TreeNode> 
    AdaptiveImageTree::copyTreeRecursive(const TreeNode* sourceNode) {
        if (!sourceNode) return nullptr;
        
        auto newNode = std::make_unique<TreeNode>(sourceNode->region, sourceNode->averageColor);
        
        if (sourceNode->leftChild) {
            newNode->leftChild = copyTreeRecursive(sourceNode->leftChild.get());
        }
        if (sourceNode->rightChild) {
            newNode->rightChild = copyTreeRecursive(sourceNode->rightChild.get());
        }
        
        return newNode;
    }

    std::pair<int, int> AdaptiveImageTree::getImageDimensions() const {
        return std::make_pair(imageWidth_, imageHeight_);
    }

    size_t AdaptiveImageTree::countLeafNodes() const {
        return countLeafNodesRecursive(rootNode_.get());
    }

    size_t AdaptiveImageTree::countLeafNodesRecursive(const TreeNode* node) const {
        if (!node) return 0;
        
        // If this region didn't get split, it's a leaf
        if (!node->leftChild && !node->rightChild) {
            return 1;
        }
        
        // Otherwise, count leaves in both child branches
        return countLeafNodesRecursive(node->leftChild.get()) + 
               countLeafNodesRecursive(node->rightChild.get());
    }

    double AdaptiveImageTree::getCompressionRatio() const {
        size_t totalPixels = static_cast<size_t>(imageWidth_) * imageHeight_;
        size_t leafNodes = countLeafNodes();
        
        if (totalPixels == 0) return 0.0;
        
        // How many regions we ended up with compared to original pixels
        // Smaller number = more compression (fewer regions = more simplified)
        return static_cast<double>(leafNodes) / totalPixels;
    }

    void AdaptiveImageTree::pruneTree(const PruningConfig& config) {
        if (rootNode_) {
            pruneNodeRecursive(rootNode_, config);
        }
    }

    void AdaptiveImageTree::pruneNodeRecursive(std::unique_ptr<TreeNode>& node, 
                                              const PruningConfig& config) {
        if (!node) return;
        
        // If this region is already unsplit, nothing to do
        if (!node->leftChild && !node->rightChild) {
            return;
        }
        
        // First, prune the child branches
        if (node->leftChild) {
            pruneNodeRecursive(node->leftChild, config);
        }
        if (node->rightChild) {
            pruneNodeRecursive(node->rightChild, config);
        }
        
        // Now check if we can merge this whole branch into one region
        if (shouldPruneSubtree(node.get(), config)) {
            // Throw away the children - this becomes a single region
            node->leftChild.reset();
            node->rightChild.reset();
        }
    }

    bool AdaptiveImageTree::shouldPruneSubtree(const TreeNode* node, 
                                              const PruningConfig& config) const {
        if (!node || (!node->leftChild && !node->rightChild)) {
            return false; // Nothing to prune here
        }
        
        // Count how many pixels in this branch are similar to the average color
        int totalPixels = 0;
        int similarPixels = countSimilarPixels(node, node->averageColor, 
                                             config.colorToleranceThreshold, totalPixels);
        
        if (totalPixels == 0) return false;
        
        // If most pixels are similar enough, we can merge this whole branch
        double similarityPercentage = static_cast<double>(similarPixels) / totalPixels;
        return similarityPercentage >= config.minimumSimilarityPercentage;
    }

    int AdaptiveImageTree::countSimilarPixels(const TreeNode* node, 
                                            const Utils::HSLAPixel& referenceColor,
                                            double tolerance, 
                                            int& totalPixels) const {
        if (!node) {
            return 0;
        }
        
        // If this region is unsplit, check if its color is close enough
        if (!node->leftChild && !node->rightChild) {
            int regionArea = (node->region.lowerRight.first - node->region.upperLeft.first + 1) *
                           (node->region.lowerRight.second - node->region.upperLeft.second + 1);
            totalPixels += regionArea;
            
            double colorDistance = calculateColorDistance(node->averageColor, referenceColor);
            if (colorDistance <= tolerance) {
                return regionArea;  // All pixels in this region count as similar
            } else {
                return 0;  // None of them are similar enough
            }
        }
        
        // For split regions, check both halves
        int similarCount = 0;
        if (node->leftChild) {
            similarCount += countSimilarPixels(node->leftChild.get(), referenceColor, 
                                             tolerance, totalPixels);
        }
        if (node->rightChild) {
            similarCount += countSimilarPixels(node->rightChild.get(), referenceColor, 
                                             tolerance, totalPixels);
        }
        
        return similarCount;
    }

    double AdaptiveImageTree::calculateColorDistance(const Utils::HSLAPixel& color1,
                                                   const Utils::HSLAPixel& color2) const {
        // Figure out how different two colors look to human eyes
        // Hue wraps around (red is both 0 and 360 degrees)
        
        double hueDiff = std::abs(color1.hue - color2.hue);
        if (hueDiff > 180.0) {
            hueDiff = 360.0 - hueDiff; // Go the short way around the color wheel
        }
        hueDiff /= 180.0; // Scale it down
        
        double satDiff = color1.saturation - color2.saturation;
        double lumDiff = color1.luminance - color2.luminance;
        
        // Good old Pythagorean theorem in 3D color space
        return std::sqrt(hueDiff * hueDiff + satDiff * satDiff + lumDiff * lumDiff);
    }

} // namespace ImageCompression 