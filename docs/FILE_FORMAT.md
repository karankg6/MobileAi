# .ultra File Format Specification

Version 1.0

## Overview

The `.ultra` file format is a custom binary format designed for storing ultrasound frame data. It is optimized for:
- Efficient streaming and parsing
- Metadata integrity (CRC checksums)
- Cross-platform compatibility
- Future extensibility

## File Structure

Each `.ultra` file contains:
1. **Header** (32 bytes) - Metadata and validation
2. **Payload** (variable) - Image data

```
┌─────────────────────────────────┐
│  Header (32 bytes)              │
├─────────────────────────────────┤
│  Image Data (W × H × D × BPV)  │
└─────────────────────────────────┘
```

## Header Format (32 bytes)

All multi-byte values are stored in **little-endian** format.

| Offset | Size | Type     | Field         | Description                          |
|--------|------|----------|---------------|--------------------------------------|
| 0      | 4    | uint32_t | Magic ID      | 0x554C5452 ("ULTR" in ASCII)        |
| 4      | 2    | uint16_t | Width         | Image width in pixels (128)         |
| 6      | 2    | uint16_t | Height        | Image height in pixels (128)        |
| 8      | 2    | uint16_t | Depth         | Image depth/slices (1 for 2D)       |
| 10     | 2    | uint16_t | BytesPerVoxel | Bytes per pixel (1 for uint8)       |
| 12     | 4    | uint32_t | FrameNumber   | Sequential frame number             |
| 16     | 8    | uint64_t | Timestamp     | Microseconds since epoch            |
| 24     | 4    | uint32_t | Reserved      | Reserved for future use (0)         |
| 28     | 4    | uint32_t | CRC32         | CRC32 checksum (see below)          |

### Field Details

#### Magic ID (0x554C5452)
- **Purpose**: File type identification
- **Value**: `0x554C5452` = "ULTR" in ASCII
- **Validation**: First check when parsing
- **Endianness**: Little-endian (52 4C 55 54 in hex dump)

#### Width, Height, Depth
- **Current values**: 128 × 128 × 1
- **Range**: 1-65535 (uint16_t)
- **Note**: Current implementation uses 2D (depth=1)

#### BytesPerVoxel
- **Current value**: 1 (uint8 grayscale)
- **Future options**:
  - 2: uint16 (16-bit depth)
  - 4: float32 (HDR imaging)
  - 3: RGB color (rarely used in ultrasound)

#### FrameNumber
- **Purpose**: Sequence tracking
- **Range**: 0 to 4,294,967,295
- **Usage**: Detect dropped frames, ordering

#### Timestamp
- **Format**: Microseconds since Unix epoch
- **Precision**: 1 microsecond (1e-6 seconds)
- **Example**: 1762992777297161 = Nov 13, 2025, 00:12:57.297161
- **Usage**: Synchronization, playback speed calculation

#### Reserved
- **Current value**: 0
- **Future uses**:
  - Compression flags
  - Color space indicators
  - Processing hints

#### CRC32
- **Algorithm**: CRC-32 (same as in ZIP, PNG)
- **Calculated over**: All data from offset 28 onwards (payload only)
- **Purpose**: Data integrity validation
- **Libraries**: zlib's `crc32()` function

## Image Data Format

### Current Format (V1.0)

- **Type**: Grayscale uint8
- **Range**: 0-255 (0 = black, 255 = white)
- **Size**: Width × Height × Depth × BytesPerVoxel bytes
- **Layout**: Row-major order (standard for images)

```
Pixel[0,0], Pixel[1,0], ..., Pixel[W-1,0],
Pixel[0,1], Pixel[1,1], ..., Pixel[W-1,1],
...
Pixel[0,H-1], Pixel[1,H-1], ..., Pixel[W-1,H-1]
```

### Size Calculation

For current format (128 × 128 × 1 × 1):
- **Image data**: 16,384 bytes (16 KB)
- **Total file**: 16,416 bytes (32 + 16,384)

## Parsing Algorithm

### C/C++ Example

```cpp
#include <cstdint>
#include <fstream>
#include <vector>

struct UltraHeader {
    uint32_t magicId;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t bytesPerVoxel;
    uint32_t frameNumber;
    uint64_t timestamp;
    uint32_t reserved;
    uint32_t crc;
};

bool parseUltraFile(const char* filename,
                    UltraHeader& header,
                    std::vector<uint8_t>& imageData) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    // Read header
    file.read(reinterpret_cast<char*>(&header), sizeof(UltraHeader));

    // Validate magic ID
    if (header.magicId != 0x554C5452) {
        return false;
    }

    // Read image data
    size_t dataSize = header.width * header.height *
                      header.depth * header.bytesPerVoxel;
    imageData.resize(dataSize);
    file.read(reinterpret_cast<char*>(imageData.data()), dataSize);

    return true;
}
```

### Python Example

```python
import struct

def parse_ultra_file(filename):
    with open(filename, 'rb') as f:
        # Read header (32 bytes)
        header_data = f.read(32)

        # Unpack header (little-endian)
        magic, width, height, depth, bpv, frame_num, timestamp, reserved, crc = \
            struct.unpack('<IHHHHIQII', header_data)

        # Validate magic
        assert magic == 0x554C5452, "Invalid magic ID"

        # Read image data
        data_size = width * height * depth * bpv
        image_data = f.read(data_size)

        return {
            'width': width,
            'height': height,
            'frameNumber': frame_num,
            'timestamp': timestamp,
            'imageData': image_data
        }
```

## CRC Validation

### Calculating CRC32

```cpp
#include <zlib.h>

uint32_t calculateCRC(const uint8_t* data, size_t length) {
    return crc32(0L, data, length);
}

bool validateFile(const std::vector<uint8_t>& fileData, uint32_t expectedCRC) {
    // CRC is calculated from offset 28 onwards
    uint32_t calculatedCRC = calculateCRC(fileData.data() + 28,
                                          fileData.size() - 28);
    return calculatedCRC == expectedCRC;
}
```

## Generating .ultra Files

Use the provided Python script:

```bash
cd dummy_data
python3 generate_ultra_files.py
```

This generates realistic ultrasound-like frames with:
- Speckle noise (characteristic of ultrasound)
- Depth-dependent intensity
- Anatomical-like structures
- Frame-to-frame motion simulation

## Future Extensions

### Planned Enhancements

1. **Compression**
   - Use Reserved field for compression flag
   - Add compressed size to header
   - Support zlib, LZ4, or custom codecs

2. **Multi-slice 3D**
   - Increase Depth > 1
   - Add slice spacing metadata

3. **Color Doppler**
   - BytesPerVoxel = 3 (RGB) or 4 (RGBA)
   - Color overlay on grayscale

4. **Metadata Extension**
   - Extend header to 64 bytes
   - Add probe type, frequency, depth setting
   - Patient information (anonymized)

5. **Streaming Container**
   - Multiple frames in single file
   - Frame index table
   - Seek support

## Compatibility Notes

- **Endianness**: Little-endian (Intel/ARM standard)
- **Alignment**: Packed struct (no padding)
- **Portability**: Works on x86, x64, ARM, ARM64
- **File system**: Compatible with all major OS

## Validation Checklist

When implementing a parser:

- [ ] Check magic ID (0x554C5452)
- [ ] Validate width, height > 0
- [ ] Verify file size matches header dimensions
- [ ] Validate CRC32 checksum
- [ ] Handle endianness correctly
- [ ] Check for out-of-memory conditions
- [ ] Validate timestamp is reasonable

## Tools

### Hex Dump Example

```bash
xxd -l 32 frame_0000.ultra
```

Output:
```
00000000: 5254 4c55 8000 8000 0100 0100 0000 0000  RTLU............
00000010: 1971 6597 c599 0001 0000 0000 fb2e 4cb4  .qe...........L.
```

Breakdown:
- `5254 4c55` = "RTLU" (reversed due to little-endian)
- `8000 8000` = width=128, height=128
- `0100 0100` = depth=1, bpv=1
- `0000 0000` = frameNumber=0
- CRC at the end

## References

- CRC-32: https://en.wikipedia.org/wiki/Cyclic_redundancy_check
- Binary file formats: https://en.wikipedia.org/wiki/Binary_file
- DICOM standard (for comparison): https://www.dicomstandard.org/

---

*Last updated: November 2025*
