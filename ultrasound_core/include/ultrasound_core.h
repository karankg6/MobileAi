#ifndef ULTRASOUND_CORE_H
#define ULTRASOUND_CORE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

namespace ultrasound {

/**
 * Ultrasound frame header structure (32 bytes)
 * Matches the .ultra file format specification
 */
struct UltraHeader {
    uint32_t magicId;           // 0x554C5452 ("ULTR")
    uint16_t width;             // 128
    uint16_t height;            // 128
    uint16_t depth;             // 1
    uint16_t bytesPerVoxel;     // 1
    uint32_t frameNumber;       // Sequential frame number
    uint64_t timestamp;         // Microseconds
    uint32_t reserved;          // Reserved for future use
    uint32_t crc;               // CRC32 checksum

    // Header constants
    static constexpr uint32_t MAGIC_ID = 0x554C5452;
    static constexpr size_t HEADER_SIZE = 32;

    // Validation
    bool isValid() const;
    size_t getFrameDataSize() const;
};

/**
 * Ultrasound frame data with metadata
 */
struct UltraFrame {
    UltraHeader header;
    cv::Mat frame;              // OpenCV Mat (grayscale)

    bool isValid() const { return !frame.empty() && header.isValid(); }
};

/**
 * Processing parameters for ultrasound frames
 */
struct ProcessingParams {
    // Preprocessing options
    bool normalize = true;          // Normalize intensity
    bool denoise = true;            // Apply denoising
    float gain = 1.0f;              // Intensity gain (0.5 - 2.0)

    // Filter options
    bool gaussianBlur = false;      // Apply Gaussian blur
    int blurKernelSize = 3;         // Kernel size for blur (odd number)

    // Edge enhancement
    bool sharpen = false;           // Apply sharpening
    float sharpenAmount = 1.0f;     // Sharpening strength

    // Contrast
    float contrastAlpha = 1.0f;     // Contrast multiplier (1.0 = no change)
    int contrastBeta = 0;           // Brightness offset

    // Future AI/TFLite processing hooks
    bool enableAiPreprocessing = false;

    // Default constructor with safe defaults
    ProcessingParams() = default;
};

/**
 * Main Ultrasound SDK class
 */
class UltrasoundSDK {
public:
    UltrasoundSDK();
    ~UltrasoundSDK();

    /**
     * Load frame from raw bytes (typically from file or stream)
     * @param data Raw byte data including header and frame data
     * @return UltraFrame containing header and OpenCV Mat
     */
    UltraFrame loadFrameFromBytes(const std::vector<uint8_t>& data);

    /**
     * Load frame directly from file
     * @param filepath Path to .ultra file
     * @return UltraFrame containing header and OpenCV Mat
     */
    UltraFrame loadFrameFromFile(const std::string& filepath);

    /**
     * Apply preprocessing filters to frame
     * @param frame Input frame (8-bit grayscale)
     * @param params Processing parameters
     * @return Processed frame
     */
    cv::Mat applyFilters(const cv::Mat& frame, const ProcessingParams& params = ProcessingParams());

    /**
     * Validate CRC32 checksum
     * @param data Frame data (excluding header CRC field)
     * @param expectedCrc Expected CRC value
     * @return true if CRC matches
     */
    bool validateCrc(const std::vector<uint8_t>& data, uint32_t expectedCrc);

    /**
     * Convert frame to different bit depth or color space
     * @param frame Input frame
     * @param targetDepth Target bit depth (CV_8U, CV_16U, CV_32F)
     * @return Converted frame
     */
    cv::Mat convertFrame(const cv::Mat& frame, int targetDepth);

    /**
     * Get SDK version
     * @return Version string
     */
    static std::string getVersion();

    /**
     * Future: AI/TensorFlow Lite inference hook
     * This method is a placeholder for future AI processing
     * @param frame Input frame
     * @param modelPath Path to TFLite model
     * @return Processed frame with AI inference results
     */
    cv::Mat runAiInference(const cv::Mat& frame, const std::string& modelPath);

private:
    // Internal helper methods
    UltraHeader parseHeader(const uint8_t* data);
    cv::Mat extractFrameData(const uint8_t* data, const UltraHeader& header);
    uint32_t calculateCrc32(const uint8_t* data, size_t length);

    // Processing helpers
    cv::Mat normalizeIntensity(const cv::Mat& frame);
    cv::Mat applyDenoise(const cv::Mat& frame);
    cv::Mat applyGain(const cv::Mat& frame, float gain);
    cv::Mat applySharpen(const cv::Mat& frame, float amount);

    // Internal state
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace ultrasound

#endif // ULTRASOUND_CORE_H
