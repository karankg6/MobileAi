# Ultrasound SDK API Documentation

Version 1.0.0

## Overview

The Ultrasound Core SDK provides a cross-platform C++ library for processing ultrasound imaging data. It features OpenCV integration, customizable filters, and preparation for AI/TensorFlow Lite inference.

## Architecture

```
┌─────────────────────────────────────────┐
│         Application Layer               │
│  (Android Kotlin / iOS Swift / Desktop) │
└─────────────────┬───────────────────────┘
                  │
         ┌────────▼────────┐
         │   JNI Bridge    │  (Platform-specific)
         └────────┬────────┘
                  │
         ┌────────▼────────────────┐
         │  Ultrasound Core SDK    │  (Cross-platform C++)
         │  - Frame loading        │
         │  - Image processing     │
         │  - OpenCV integration   │
         └────────┬────────────────┘
                  │
         ┌────────▼────────┐
         │    OpenCV       │
         └─────────────────┘
```

## Core Classes

### UltrasoundSDK

Main SDK class providing all ultrasound processing functionality.

#### Constructor

```cpp
UltrasoundSDK();
```

**Description**: Initialize the SDK.
**Thread-safety**: Thread-safe
**Exceptions**: May throw `std::runtime_error` on initialization failure

#### Destructor

```cpp
~UltrasoundSDK();
```

**Description**: Clean up SDK resources.

---

### Frame Loading

#### loadFrameFromBytes

```cpp
UltraFrame loadFrameFromBytes(const std::vector<uint8_t>& data);
```

**Parameters**:
- `data`: Raw byte array containing .ultra file data (header + image)

**Returns**: `UltraFrame` struct containing header and OpenCV Mat

**Exceptions**:
- `std::runtime_error`: If data is invalid or corrupted
- `std::runtime_error`: If header validation fails

**Example**:
```cpp
UltrasoundSDK sdk;

// Read file into vector
std::vector<uint8_t> fileData = readFile("frame_0000.ultra");

// Load frame
UltraFrame frame = sdk.loadFrameFromBytes(fileData);

// Access frame data
std::cout << "Frame: " << frame.header.frameNumber << std::endl;
std::cout << "Size: " << frame.frame.cols << "x" << frame.frame.rows << std::endl;
```

#### loadFrameFromFile

```cpp
UltraFrame loadFrameFromFile(const std::string& filepath);
```

**Parameters**:
- `filepath`: Path to .ultra file

**Returns**: `UltraFrame` struct

**Exceptions**:
- `std::runtime_error`: If file doesn't exist or can't be read

**Example**:
```cpp
UltrasoundSDK sdk;
UltraFrame frame = sdk.loadFrameFromFile("dummy_data/frame_0000.ultra");
```

---

### Image Processing

#### applyFilters

```cpp
cv::Mat applyFilters(const cv::Mat& frame,
                     const ProcessingParams& params = ProcessingParams());
```

**Parameters**:
- `frame`: Input OpenCV Mat (8-bit grayscale)
- `params`: Processing parameters (optional)

**Returns**: Processed OpenCV Mat

**Exceptions**:
- `std::runtime_error`: If frame is empty

**Example**:
```cpp
UltrasoundSDK sdk;
UltraFrame frame = sdk.loadFrameFromFile("frame_0000.ultra");

// Create processing parameters
ProcessingParams params;
params.normalize = true;
params.denoise = true;
params.gain = 1.2f;
params.sharpen = true;

// Apply filters
cv::Mat processed = sdk.applyFilters(frame.frame, params);

// Display or save
cv::imshow("Ultrasound", processed);
cv::imwrite("output.png", processed);
```

#### convertFrame

```cpp
cv::Mat convertFrame(const cv::Mat& frame, int targetDepth);
```

**Parameters**:
- `frame`: Input OpenCV Mat
- `targetDepth`: Target bit depth (CV_8U, CV_16U, CV_32F)

**Returns**: Converted Mat

**Example**:
```cpp
// Convert to 16-bit
cv::Mat frame16 = sdk.convertFrame(frame.frame, CV_16U);

// Convert to float32 for HDR processing
cv::Mat frameFloat = sdk.convertFrame(frame.frame, CV_32F);
```

---

### Validation

#### validateCrc

```cpp
bool validateCrc(const std::vector<uint8_t>& data, uint32_t expectedCrc);
```

**Parameters**:
- `data`: Full file data
- `expectedCrc`: Expected CRC from header

**Returns**: `true` if CRC matches, `false` otherwise

**Example**:
```cpp
UltrasoundSDK sdk;
std::vector<uint8_t> fileData = readFile("frame_0000.ultra");

// Extract CRC from header (at offset 28)
uint32_t crc;
memcpy(&crc, fileData.data() + 28, sizeof(uint32_t));

// Validate
if (sdk.validateCrc(fileData, crc)) {
    std::cout << "CRC OK" << std::endl;
} else {
    std::cerr << "CRC mismatch!" << std::endl;
}
```

---

### Utility Methods

#### getVersion

```cpp
static std::string getVersion();
```

**Returns**: SDK version string (e.g., "1.0.0")

**Example**:
```cpp
std::cout << "SDK Version: " << UltrasoundSDK::getVersion() << std::endl;
```

---

### Future AI Integration

#### runAiInference

```cpp
cv::Mat runAiInference(const cv::Mat& frame, const std::string& modelPath);
```

**Status**: Not yet implemented (placeholder)

**Purpose**: Run TensorFlow Lite inference on frame

**Future usage**:
```cpp
// This will work in a future version
cv::Mat result = sdk.runAiInference(frame.frame, "model.tflite");
```

---

## Data Structures

### UltraHeader

```cpp
struct UltraHeader {
    uint32_t magicId;           // 0x554C5452 ("ULTR")
    uint16_t width;             // Image width (128)
    uint16_t height;            // Image height (128)
    uint16_t depth;             // Depth (1 for 2D)
    uint16_t bytesPerVoxel;     // Bytes per pixel (1)
    uint32_t frameNumber;       // Frame sequence number
    uint64_t timestamp;         // Timestamp in microseconds
    uint32_t reserved;          // Reserved for future use
    uint32_t crc;               // CRC32 checksum

    // Methods
    bool isValid() const;       // Validate header
    size_t getFrameDataSize() const;  // Calculate data size
};
```

**Constants**:
- `UltraHeader::MAGIC_ID = 0x554C5452`
- `UltraHeader::HEADER_SIZE = 32`

**Example**:
```cpp
UltraFrame frame = sdk.loadFrameFromFile("frame_0000.ultra");

if (frame.header.isValid()) {
    std::cout << "Width: " << frame.header.width << std::endl;
    std::cout << "Height: " << frame.header.height << std::endl;
    std::cout << "Frame #: " << frame.header.frameNumber << std::endl;
    std::cout << "Timestamp: " << frame.header.timestamp << " µs" << std::endl;
}
```

### UltraFrame

```cpp
struct UltraFrame {
    UltraHeader header;   // Frame metadata
    cv::Mat frame;        // OpenCV Mat (grayscale)

    bool isValid() const; // Check if frame is valid
};
```

### ProcessingParams

```cpp
struct ProcessingParams {
    // Preprocessing
    bool normalize = true;          // Normalize intensity (0-255)
    bool denoise = true;            // Apply denoising
    float gain = 1.0f;              // Intensity gain (0.5 - 2.0)

    // Filtering
    bool gaussianBlur = false;      // Apply Gaussian blur
    int blurKernelSize = 3;         // Kernel size (odd number)

    // Enhancement
    bool sharpen = false;           // Apply sharpening
    float sharpenAmount = 1.0f;     // Sharpening strength

    // Contrast
    float contrastAlpha = 1.0f;     // Contrast multiplier
    int contrastBeta = 0;           // Brightness offset

    // Future
    bool enableAiPreprocessing = false;
};
```

**Example configurations**:

```cpp
// Preset 1: Clean (minimal processing)
ProcessingParams cleanParams;
cleanParams.normalize = true;
cleanParams.denoise = false;
cleanParams.gain = 1.0f;

// Preset 2: Enhanced (maximum clarity)
ProcessingParams enhancedParams;
enhancedParams.normalize = true;
enhancedParams.denoise = true;
enhancedParams.gain = 1.5f;
enhancedParams.sharpen = true;
enhancedParams.sharpenAmount = 0.5f;
enhancedParams.contrastAlpha = 1.2f;

// Preset 3: Diagnostic (balanced)
ProcessingParams diagnosticParams;
diagnosticParams.normalize = true;
diagnosticParams.denoise = true;
diagnosticParams.gain = 1.2f;
diagnosticParams.gaussianBlur = false;
diagnosticParams.contrastAlpha = 1.1f;
```

---

## Android JNI Bridge API

### UltrasoundNative (Kotlin)

Kotlin wrapper for JNI calls.

#### Constructor

```kotlin
val sdk = UltrasoundNative()
```

#### Methods

##### processFrame

```kotlin
fun processFrame(
    data: ByteArray,
    normalize: Boolean = true,
    denoise: Boolean = true,
    gain: Float = 1.0f
): Long
```

**Returns**: OpenCV Mat address (pointer as Long)

**Usage**:
```kotlin
val frameData = File("frame_0000.ultra").readBytes()
val matAddress = sdk.processFrame(frameData, normalize = true, denoise = true, gain = 1.2f)
```

##### getFrameMetadata

```kotlin
fun getFrameMetadata(data: ByteArray): FrameMetadata
```

**Returns**: `FrameMetadata(frameNumber: Int, timestamp: Long)`

##### matToBitmap

```kotlin
fun matToBitmap(matAddress: Long, bitmap: Bitmap)
```

**Usage**:
```kotlin
val bitmap = Bitmap.createBitmap(128, 128, Bitmap.Config.ARGB_8888)
sdk.matToBitmap(matAddress, bitmap)
```

##### releaseMat

```kotlin
fun releaseMat(matAddress: Long)
```

**Important**: Always call this to prevent memory leaks!

```kotlin
val matAddress = sdk.processFrame(data)
// ... use mat ...
sdk.releaseMat(matAddress)  // Don't forget!
```

##### getVersion

```kotlin
fun getVersion(): String
```

---

## Complete Usage Example

### C++ (Desktop/Testing)

```cpp
#include "ultrasound_core.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    using namespace ultrasound;

    try {
        // Initialize SDK
        UltrasoundSDK sdk;
        std::cout << "SDK Version: " << sdk.getVersion() << std::endl;

        // Load frame
        UltraFrame frame = sdk.loadFrameFromFile("dummy_data/frame_0000.ultra");

        // Print metadata
        std::cout << "Frame: " << frame.header.frameNumber << std::endl;
        std::cout << "Timestamp: " << frame.header.timestamp << " µs" << std::endl;
        std::cout << "Size: " << frame.frame.cols << "x" << frame.frame.rows << std::endl;

        // Process frame
        ProcessingParams params;
        params.normalize = true;
        params.denoise = true;
        params.gain = 1.2f;

        cv::Mat processed = sdk.applyFilters(frame.frame, params);

        // Display
        cv::imshow("Ultrasound", processed);
        cv::waitKey(0);

        // Save
        cv::imwrite("output.png", processed);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

### Kotlin (Android with Flow)

```kotlin
class UltrasoundRepository(private val context: Context) {
    private val sdk = UltrasoundNative()

    fun streamFrames(files: List<File>): Flow<UltrasoundFrame> = flow {
        for (file in files) {
            val data = file.readBytes()
            val metadata = sdk.getFrameMetadata(data)
            val matAddress = sdk.processFrame(data, normalize = true, denoise = true, gain = 1.2f)

            val bitmap = Bitmap.createBitmap(128, 128, Bitmap.Config.ARGB_8888)
            sdk.matToBitmap(matAddress, bitmap)
            sdk.releaseMat(matAddress)

            emit(UltrasoundFrame(bitmap, metadata.frameNumber, metadata.timestamp, file.name))
            delay(1000) // 1 FPS
        }
    }.flowOn(Dispatchers.IO)
}
```

---

## Thread Safety

- ✅ **Thread-safe**: `loadFrameFromBytes()`, `loadFrameFromFile()`
- ✅ **Thread-safe**: `applyFilters()`
- ✅ **Thread-safe**: JNI bridge (uses mutex)
- ⚠️ **Not thread-safe**: OpenCV Mat objects (use copies)

**Best practice**: Process frames in background thread (Dispatchers.IO), display on main thread.

---

## Performance Considerations

### Memory Usage

- Each frame: ~16 KB (128×128 uint8)
- Processing overhead: ~32 KB temporary buffers
- OpenCV operations: In-place when possible

### Processing Time (typical)

- Load frame: < 1 ms
- Normalize: < 1 ms
- Denoise: 5-10 ms (most expensive)
- Sharpen: 2-3 ms
- Total: 10-15 ms per frame

**Recommendation**: For real-time (30 FPS), disable denoising or use GPU acceleration.

---

## Error Handling

All SDK methods may throw `std::runtime_error`. Always use try-catch:

```cpp
try {
    UltraFrame frame = sdk.loadFrameFromFile(path);
} catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

In Kotlin (JNI), exceptions are converted to Java RuntimeException.

---

## Building the SDK

See [SETUP.md](SETUP.md) for build instructions.

---

*Last updated: November 2025*
