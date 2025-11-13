#ifndef ULTRASOUND_JNI_H
#define ULTRASOUND_JNI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the Ultrasound SDK
 * @return SDK handle (pointer)
 */
JNIEXPORT jlong JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeInit(
    JNIEnv* env, jobject thiz);

/**
 * Release the Ultrasound SDK
 * @param handle SDK handle
 */
JNIEXPORT void JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeRelease(
    JNIEnv* env, jobject thiz, jlong handle);

/**
 * Load and process frame from byte array
 * @param handle SDK handle
 * @param data Byte array containing .ultra file data
 * @param normalize Apply normalization
 * @param denoise Apply denoising
 * @param gain Intensity gain
 * @return OpenCV Mat address (pointer)
 */
JNIEXPORT jlong JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeProcessFrame(
    JNIEnv* env, jobject thiz, jlong handle, jbyteArray data,
    jboolean normalize, jboolean denoise, jfloat gain);

/**
 * Get frame metadata
 * @param handle SDK handle
 * @param data Byte array containing .ultra file data
 * @return Long array [frameNumber, timestamp]
 */
JNIEXPORT jlongArray JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeGetFrameMetadata(
    JNIEnv* env, jobject thiz, jlong handle, jbyteArray data);

/**
 * Convert Mat to Android Bitmap
 * @param matAddress OpenCV Mat address
 * @param bitmap Android Bitmap object
 */
JNIEXPORT void JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeMatToBitmap(
    JNIEnv* env, jobject thiz, jlong matAddress, jobject bitmap);

/**
 * Release Mat memory
 * @param matAddress OpenCV Mat address
 */
JNIEXPORT void JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeReleaseMat(
    JNIEnv* env, jobject thiz, jlong matAddress);

/**
 * Get SDK version
 * @return Version string
 */
JNIEXPORT jstring JNICALL
Java_org_kgajjar_mobileai_ultrasound_UltrasoundNative_nativeGetVersion(
    JNIEnv* env, jobject thiz);

#ifdef __cplusplus
}
#endif

#endif // ULTRASOUND_JNI_H
