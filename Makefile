# Content-Aware Image Compression Tool
# Modern C++17 build system
#
# Usage:
#   make           - Build the compression tool
#   make clean     - Remove all built files
#   make install   - Install to /usr/local/bin (requires sudo)

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=native -flto -DNDEBUG -ffast-math -funroll-loops
INCLUDES = -Iinclude
LDFLAGS = -flto -O3

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Target executable
TARGET = compress

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/core/ImageCompressor.cpp \
          $(SRC_DIR)/core/AdaptiveImageTree.cpp \
          $(SRC_DIR)/statistics/ImageStatistics.cpp \
          $(SRC_DIR)/utils/image/HSLAPixel.cpp \
          $(SRC_DIR)/utils/image/ColorConversion.cpp \
          $(SRC_DIR)/utils/image/PNG.cpp \
          $(SRC_DIR)/utils/external/lodepng/lodepng.cpp

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Build directories
BUILD_DIRS = $(BUILD_DIR) \
             $(BUILD_DIR)/core \
             $(BUILD_DIR)/statistics \
             $(BUILD_DIR)/utils/image \
             $(BUILD_DIR)/utils/external \
             $(BUILD_DIR)/utils/external/lodepng

.PHONY: all clean install help

all: $(TARGET)

$(TARGET): $(BUILD_DIRS) $(OBJECTS)
	@echo "Linking $(TARGET)..."
	@$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "✓ Build complete: ./$(TARGET)"

# Create build directories
$(BUILD_DIRS):
	@mkdir -p $@

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "✓ Clean complete"

install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin..."
	@sudo cp $(TARGET) /usr/local/bin/
	@echo "✓ Installation complete"

help:
	@echo "Content-Aware Image Compression Tool"
	@echo "===================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all (default) - Build the compression tool"
	@echo "  clean         - Remove all built files"
	@echo "  install       - Install to /usr/local/bin (requires sudo)"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Usage after building:"
	@echo "  ./$(TARGET) <input_dir> <output_dir> [quality]"
	@echo ""
	@echo "Example:"
	@echo "  ./$(TARGET) ./photos ./compressed medium" 