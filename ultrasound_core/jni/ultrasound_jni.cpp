#include "ultrasound_jni.h"
#include "ultrasound_core.h"
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <vector>
#include <mutex>

using namespace ultrasound;

// Thread-safety mutex for SDK operations
static std::mutex g_sdkMutex;

// Helper function to convert jbyteArray to std::vector<uint8_t>
static std::vector<uint8_t> jbyteArrayToVector(JNIEnv* env, jbyteArray array) {
    jsize length = env->GetArrayLength(array);
    std::vector<uint8_t> result(length);

    jbyte* bytes = env->GetByteArrayElements(array, nullptr);
    if (bytes) {
        memcpy(result.data(), bytes, length);
        env->ReleaseByteArrayElements(array, bytes, JNI_ABORT);
    }

    return result;
}

// Helper function to safely get SDK pointer
static UltrasoundSDK* getSDK(jlong handle) {
    return reinterpret_cast<UltrasoundSDK*>(handle);
}

// Helper function to safely get Mat pointer
static cv::Mat* getMat(jlong matAddress) {
    return reinterpret_cast<cv::Mat*>(matAddress);
}

//==============================================================================
// JNI Implementation
//==============================================================================

JNIEXPORT jlong JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeInit(
    JNIEnv* env, jobject thiz) {

    std::lock_guard<std::mutex> lock(g_sdkMutex);

    try {
        UltrasoundSDK* sdk = new UltrasoundSDK();
        return reinterpret_cast<jlong>(sdk);
    } catch (const std::exception& e) {
        // Throw Java exception
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        if (exClass) {
            env->ThrowNew(exClass, e.what());
        }
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeRelease(
    JNIEnv* env, jobject thiz, jlong handle) {

    std::lock_guard<std::mutex> lock(g_sdkMutex);

    UltrasoundSDK* sdk = getSDK(handle);
    if (sdk) {
        delete sdk;
    }
}

JNIEXPORT jlong JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeProcessFrame(
    JNIEnv* env, jobject thiz, jlong handle, jbyteArray data,
    jboolean normalize, jboolean denoise, jfloat gain) {

    std::lock_guard<std::mutex> lock(g_sdkMutex);

    UltrasoundSDK* sdk = getSDK(handle);
    if (!sdk) {
        jclass exClass = env->FindClass("java/lang/IllegalArgumentException");
        if (exClass) {
            env->ThrowNew(exClass, "Invalid SDK handle");
        }
        return 0;
    }

    try {
        // Convert Java byte array to C++ vector
        std::vector<uint8_t> frameData = jbyteArrayToVector(env, data);

        // Load frame
        UltraFrame ultraFrame = sdk->loadFrameFromBytes(frameData);

        // Set up processing parameters
        ProcessingParams params;
        params.normalize = normalize;
        params.denoise = denoise;
        params.gain = gain;

        // Apply filters
        cv::Mat processedFrame = sdk->applyFilters(ultraFrame.frame, params);

        // Create a new Mat on heap and return its address
        cv::Mat* resultMat = new cv::Mat(processedFrame.clone());
        return reinterpret_cast<jlong>(resultMat);

    } catch (const std::exception& e) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        if (exClass) {
            env->ThrowNew(exClass, e.what());
        }
        return 0;
    }
}

JNIEXPORT jlongArray JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeGetFrameMetadata(
    JNIEnv* env, jobject thiz, jlong handle, jbyteArray data) {

    std::lock_guard<std::mutex> lock(g_sdkMutex);

    UltrasoundSDK* sdk = getSDK(handle);
    if (!sdk) {
        jclass exClass = env->FindClass("java/lang/IllegalArgumentException");
        if (exClass) {
            env->ThrowNew(exClass, "Invalid SDK handle");
        }
        return nullptr;
    }

    try {
        // Convert Java byte array to C++ vector
        std::vector<uint8_t> frameData = jbyteArrayToVector(env, data);

        // Load frame
        UltraFrame ultraFrame = sdk->loadFrameFromBytes(frameData);

        // Create result array [frameNumber, timestamp]
        jlongArray result = env->NewLongArray(2);
        if (result) {
            jlong values[2];
            values[0] = static_cast<jlong>(ultraFrame.header.frameNumber);
            values[1] = static_cast<jlong>(ultraFrame.header.timestamp);
            env->SetLongArrayRegion(result, 0, 2, values);
        }

        return result;

    } catch (const std::exception& e) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        if (exClass) {
            env->ThrowNew(exClass, e.what());
        }
        return nullptr;
    }
}

JNIEXPORT void JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeMatToBitmap(
    JNIEnv* env, jobject thiz, jlong matAddress, jobject bitmap) {

    cv::Mat* mat = getMat(matAddress);
    if (!mat || mat->empty()) {
        jclass exClass = env->FindClass("java/lang/IllegalArgumentException");
        if (exClass) {
            env->ThrowNew(exClass, "Invalid Mat address or empty Mat");
        }
        return;
    }

    try {
        AndroidBitmapInfo info;
        void* pixels = nullptr;

        // Get bitmap info
        if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
            throw std::runtime_error("Failed to get bitmap info");
        }

        // Lock bitmap pixels
        if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
            throw std::runtime_error("Failed to lock bitmap pixels");
        }

        // Convert Mat to RGBA if needed
        cv::Mat rgba;
        if (mat->channels() == 1) {
            // Grayscale to RGBA
            cv::cvtColor(*mat, rgba, cv::COLOR_GRAY2RGBA);
        } else if (mat->channels() == 3) {
            cv::cvtColor(*mat, rgba, cv::COLOR_BGR2RGBA);
        } else {
            rgba = mat->clone();
        }

        // Ensure correct size
        if (rgba.cols != (int)info.width || rgba.rows != (int)info.height) {
            cv::resize(rgba, rgba, cv::Size(info.width, info.height));
        }

        // Copy data to bitmap
        memcpy(pixels, rgba.data, info.width * info.height * 4);

        // Unlock bitmap
        AndroidBitmap_unlockPixels(env, bitmap);

    } catch (const std::exception& e) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        if (exClass) {
            env->ThrowNew(exClass, e.what());
        }
    }
}

JNIEXPORT void JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeReleaseMat(
    JNIEnv* env, jobject thiz, jlong matAddress) {

    cv::Mat* mat = getMat(matAddress);
    if (mat) {
        delete mat;
    }
}

JNIEXPORT jstring JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeGetVersion(
    JNIEnv* env, jobject thiz) {

    std::string version = UltrasoundSDK::getVersion();
    return env->NewStringUTF(version.c_str());
}
