# Content-Aware Image Compression

A fast, intelligent image compression tool that uses entropy-based adaptive tree compression to preserve visual quality while reducing file sizes.

## What is this project about?

This project solves the challenge of **intelligent image compression** by:
- Using **content-aware algorithms** that preserve important details while compressing uniform areas
- Providing **continuous quality control** (0.0 to 1.0) for fine-tuned compression
- Automatically detecting high-entropy regions (complex details) and preserving them
- Compressing low-entropy regions (uniform areas) more aggressively

|               Raw Input                |                     Q = 0.7                      |                     Q = 0.5                      |                     Q = 0.4                      |                     Q = 0.3                      |                     Q = 0.2                      |
| :------------------------------------: | :----------------------------------------------: | :----------------------------------------------: | :----------------------------------------------: | :----------------------------------------------: | :----------------------------------------------: |
| ![example1](./demo/input/example1.png) | ![example1_07](./demo/output/example1_q0.70.png) | ![example1_05](./demo/output/example1_q0.50.png) | ![example1_04](./demo/output/example1_q0.40.png) | ![example1_03](./demo/output/example1_q0.30.png) | ![example1_02](./demo/output/example1_q0.20.png) |

Other examples:
|                                                  |                                                  |                                                  |                                                  |
| :----------------------------------------------: | :----------------------------------------------: | :----------------------------------------------: | :----------------------------------------------: |
| ![example2_04](./demo/output/example2_q0.40.png) | ![example3_04](./demo/output/example3_q0.40.png) | ![example5_02](./demo/output/example5_q0.20.png) | ![example6_07](./demo/output/example6_q0.70.png) |


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
Found 6 PNG file(s) to compress
Quality: 0.30 (medium)
Output directory: ./compressed

Processing: example6.png -> example6_q0.30.png ... ✓ (0.1% compression, 0.43s)
Processing: example5.png -> example5_q0.30.png ... ✓ (2.0% compression, 1.30s)
Processing: example4.png -> example4_q0.30.png ... ✓ (0.5% compression, 0.34s)
Processing: example1.png -> example1_q0.30.png ... ✓ (0.1% compression, 0.31s)
Processing: example3.png -> example3_q0.30.png ... ✓ (0.2% compression, 0.70s)
Processing: example2.png -> example2_q0.30.png ... ✓ (0.2% compression, 0.54s)

=== Compression Summary ===
Files processed: 6/6
Total processing time: 3.62 seconds
Average compression ratio: 0.6%
Average time per image: 0.60 seconds
```

### Requirements
- **C++17** compatible compiler (GCC 7+ or Clang 5+)
- **System**: macOS, Linux, or Windows with C++17 support
- **Dependencies**: All included (lodepng for PNG support)

## Quality Scale Meaning

| Quality Range | Description             | Use Case             |
| ------------- | ----------------------- | -------------------- |
| `0.0 - 0.2`   | Maximum compression     | Thumbnails, previews |
| `0.3 - 0.4`   | Aggressive compression  | Web optimization     |
| `0.5 - 0.6`   | Balanced (default: 0.5) | General purpose      |
| `0.7 - 0.8`   | High quality            | Professional images  |
| `0.9 - 1.0`   | Minimal compression     | Archival quality     |

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
