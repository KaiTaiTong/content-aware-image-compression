# Content-Aware Image Compression

A fast, intelligent image compression tool that uses entropy-based adaptive tree compression to preserve visual quality while reducing file sizes.

## What is this project about?

This project solves the challenge of **intelligent image compression** by:
- Using **content-aware algorithms** that preserve important details while compressing uniform areas
- Providing **continuous quality control** (0.0 to 1.0) for fine-tuned compression
- Automatically detecting high-entropy regions (complex details) and preserving them
- Compressing low-entropy regions (uniform areas) more aggressively

## How to Run

### Setup & Installation
```bash
# Build the tool
make

# Clean build (if needed)
make clean && make
```

### Basic Usage
```bash
./compress input_directory output_directory [quality]
```

**Examples:**
```bash
# Default quality (0.5)
./compress ./photos ./compressed

# Custom quality (0.0 = max compression, 1.0 = minimal compression)
./compress ./photos ./compressed 0.75
```

### Output Results
```
Found 3 PNG file(s) to compress
Quality: 0.75
Output directory: ./compressed

Processing: photo1.png -> photo1_q0.75.png ... ✓ (2.3% compression, 12.4s)
Processing: photo2.png -> photo2_q0.75.png ... ✓ (1.8% compression, 8.7s)
Processing: photo3.png -> photo3_q0.75.png ... ✓ (3.1% compression, 15.2s)

=== Compression Summary ===
Files processed: 3/3
Total processing time: 36.3 seconds
Average compression ratio: 2.4%
```

### Requirements
- **C++17** compatible compiler (GCC 7+ or Clang 5+)
- **System**: macOS, Linux, or Windows with C++17 support
- **Dependencies**: All included (lodepng for PNG support)

## Quality Scale Meaning

| Quality Range | Description | Use Case |
|---------------|-------------|----------|
| `0.0 - 0.2` | Maximum compression | Thumbnails, previews |
| `0.3 - 0.4` | Aggressive compression | Web optimization |
| `0.5 - 0.6` | Balanced (default: 0.5) | General purpose |
| `0.7 - 0.8` | High quality | Professional images |
| `0.9 - 1.0` | Minimal compression | Archival quality |

## Project Structure

```
image-compression/
├── src/
│   ├── main.cpp                    # Command-line interface
│   ├── core/
│   │   ├── AdaptiveImageTree.cpp   # Core compression algorithm
│   │   └── ImageCompressor.cpp     # High-level API
│   ├── statistics/
│   │   └── ImageStatistics.cpp     # Entropy and color analysis
│   └── utils/
│       ├── image/                  # Image utilities
│       └── external/               # Third-party libraries (lodepng)
├── include/                        # Header files
├── Makefile                        # Build system
└── README.md                       # This documentation
```

## Core Algorithm

The compression uses **entropy-based adaptive tree partitioning**:

1. **Adaptive Tree Construction**: Recursively partitions the image into regions
2. **Entropy-Based Splitting**: Chooses splits that minimize weighted entropy across boundaries
3. **Quality-Controlled Pruning**: Removes detail based on quality settings using exponential mapping
4. **HSL Color Space**: Uses perceptually-aware color analysis
5. **Continuous Quality**: Small quality changes (0.01) produce visible differences

**Performance**: O(n log n) time complexity, typically 0.1% - 15% compression ratios.

## License

This project is licensed under the **PolyForm Noncommercial License 1.0.0**.

- You may use, modify, and distribute this software for **noncommercial purposes only**.
- **Commercial use is not permitted**. For commercial licensing, contact the copyright holder.
- See the [LICENSE](./LICENSE) file for the full license text and legal terms.

For more information, visit: https://polyformproject.org/licenses/noncommercial/1.0.0/
