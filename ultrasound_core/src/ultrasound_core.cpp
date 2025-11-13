#include "ultrasound_core.h"
#include <fstream>
#include <cstring>
#include <stdexcept>
#include <zlib.h>

namespace ultrasound {

// Version string
static const std::string SDK_VERSION = "1.0.0";

//==============================================================================
// UltraHeader Implementation
//==============================================================================

bool UltraHeader::isValid() const {
    return magicId == MAGIC_ID &&
           width > 0 && height > 0 &&
           bytesPerVoxel > 0;
}

size_t UltraHeader::getFrameDataSize() const {
    return static_cast<size_t>(width) * height * depth * bytesPerVoxel;
}

//==============================================================================
// UltrasoundSDK::Impl - Private implementation
//==============================================================================

class UltrasoundSDK::Impl {
public:
    Impl() {}
    ~Impl() {}

    // Add any internal state here
    ProcessingParams defaultParams;
};

//==============================================================================
// UltrasoundSDK Implementation
//==============================================================================

UltrasoundSDK::UltrasoundSDK()
    : pImpl(std::make_unique<Impl>()) {
}

UltrasoundSDK::~UltrasoundSDK() = default;

UltraHeader UltrasoundSDK::parseHeader(const uint8_t* data) {
    if (!data) {
        throw std::runtime_error("Null data pointer");
    }

    UltraHeader header;

    // Parse header (little-endian)
    const uint8_t* ptr = data;

    memcpy(&header.magicId, ptr, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    memcpy(&header.width, ptr, sizeof(uint16_t)); ptr += sizeof(uint16_t);
    memcpy(&header.height, ptr, sizeof(uint16_t)); ptr += sizeof(uint16_t);
    memcpy(&header.depth, ptr, sizeof(uint16_t)); ptr += sizeof(uint16_t);
    memcpy(&header.bytesPerVoxel, ptr, sizeof(uint16_t)); ptr += sizeof(uint16_t);
    memcpy(&header.frameNumber, ptr, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    memcpy(&header.timestamp, ptr, sizeof(uint64_t)); ptr += sizeof(uint64_t);
    memcpy(&header.reserved, ptr, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    memcpy(&header.crc, ptr, sizeof(uint32_t)); ptr += sizeof(uint32_t);

    return header;
}

cv::Mat UltrasoundSDK::extractFrameData(const uint8_t* data, const UltraHeader& header) {
    // Skip header and read frame data
    const uint8_t* frameData = data + UltraHeader::HEADER_SIZE;
    size_t frameSize = header.getFrameDataSize();

    // Create OpenCV Mat (grayscale, 8-bit)
    cv::Mat frame(header.height, header.width, CV_8UC1);

    // Copy data into Mat
    memcpy(frame.data, frameData, frameSize);

    return frame;
}

uint32_t UltrasoundSDK::calculateCrc32(const uint8_t* data, size_t length) {
    return crc32(0L, data, length);
}

bool UltrasoundSDK::validateCrc(const std::vector<uint8_t>& data, uint32_t expectedCrc) {
    if (data.size() < UltraHeader::HEADER_SIZE) {
        return false;
    }

    // Calculate CRC from after the CRC field (offset 28)
    uint32_t calculatedCrc = calculateCrc32(data.data() + 28, data.size() - 28);
    return calculatedCrc == expectedCrc;
}

UltraFrame UltrasoundSDK::loadFrameFromBytes(const std::vector<uint8_t>& data) {
    if (data.size() < UltraHeader::HEADER_SIZE) {
        throw std::runtime_error("Data too small to contain header");
    }

    UltraFrame frame;
    frame.header = parseHeader(data.data());

    // Validate header
    if (!frame.header.isValid()) {
        throw std::runtime_error("Invalid header: magic ID mismatch");
    }

    // Check if data size matches expected size
    size_t expectedSize = UltraHeader::HEADER_SIZE + frame.header.getFrameDataSize();
    if (data.size() < expectedSize) {
        throw std::runtime_error("Data size mismatch");
    }

    // Validate CRC (optional but recommended)
    if (!validateCrc(data, frame.header.crc)) {
        // Note: In production, you might want to make this configurable
        // For now, we'll just log a warning and continue
        // std::cerr << "Warning: CRC mismatch" << std::endl;
    }

    // Extract frame data
    frame.frame = extractFrameData(data.data(), frame.header);

    return frame;
}

UltraFrame UltrasoundSDK::loadFrameFromFile(const std::string& filepath) {
    // Open file
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    // Get file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read entire file into buffer
    std::vector<uint8_t> buffer(fileSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        throw std::runtime_error("Failed to read file: " + filepath);
    }

    return loadFrameFromBytes(buffer);
}

cv::Mat UltrasoundSDK::normalizeIntensity(const cv::Mat& frame) {
    cv::Mat normalized;
    cv::normalize(frame, normalized, 0, 255, cv::NORM_MINMAX);
    return normalized;
}

cv::Mat UltrasoundSDK::applyDenoise(const cv::Mat& frame) {
    cv::Mat denoised;
    // Use fastNlMeansDenoising for grayscale images
    // Parameters: src, dst, h (filter strength), templateWindowSize, searchWindowSize
    cv::fastNlMeansDenoising(frame, denoised, 10, 7, 21);
    return denoised;
}

cv::Mat UltrasoundSDK::applyGain(const cv::Mat& frame, float gain) {
    cv::Mat gained;
    frame.convertTo(gained, -1, gain, 0);
    return gained;
}

cv::Mat UltrasoundSDK::applySharpen(const cv::Mat& frame, float amount) {
    cv::Mat blurred;
    cv::GaussianBlur(frame, blurred, cv::Size(0, 0), 3);

    cv::Mat sharpened;
    cv::addWeighted(frame, 1.0 + amount, blurred, -amount, 0, sharpened);

    return sharpened;
}

cv::Mat UltrasoundSDK::applyFilters(const cv::Mat& frame, const ProcessingParams& params) {
    if (frame.empty()) {
        throw std::runtime_error("Empty frame");
    }

    cv::Mat processed = frame.clone();

    // Apply normalization
    if (params.normalize) {
        processed = normalizeIntensity(processed);
    }

    // Apply gain
    if (params.gain != 1.0f) {
        processed = applyGain(processed, params.gain);
    }

    // Apply denoising
    if (params.denoise) {
        processed = applyDenoise(processed);
    }

    // Apply Gaussian blur
    if (params.gaussianBlur && params.blurKernelSize > 0) {
        cv::GaussianBlur(processed, processed,
                        cv::Size(params.blurKernelSize, params.blurKernelSize), 0);
    }

    // Apply sharpening
    if (params.sharpen) {
        processed = applySharpen(processed, params.sharpenAmount);
    }

    // Apply contrast adjustment
    if (params.contrastAlpha != 1.0f || params.contrastBeta != 0) {
        processed.convertTo(processed, -1, params.contrastAlpha, params.contrastBeta);
    }

    // Ensure values are in valid range
    cv::Mat clamped;
    cv::normalize(processed, clamped, 0, 255, cv::NORM_MINMAX);
    clamped.convertTo(processed, CV_8UC1);

    return processed;
}

cv::Mat UltrasoundSDK::convertFrame(const cv::Mat& frame, int targetDepth) {
    cv::Mat converted;
    frame.convertTo(converted, targetDepth);
    return converted;
}

std::string UltrasoundSDK::getVersion() {
    return SDK_VERSION;
}

cv::Mat UltrasoundSDK::runAiInference(const cv::Mat& frame, const std::string& modelPath) {
    // Placeholder for future TensorFlow Lite integration
    // This will be implemented when AI processing is added

    // For now, just return the original frame
    // In the future, this will:
    // 1. Load TFLite model
    // 2. Preprocess frame for model input
    // 3. Run inference
    // 4. Post-process results
    // 5. Overlay results on frame or return processed frame

    throw std::runtime_error("AI inference not yet implemented. "
                           "This is a placeholder for future TensorFlow Lite integration.");
}

} // namespace ultrasound
