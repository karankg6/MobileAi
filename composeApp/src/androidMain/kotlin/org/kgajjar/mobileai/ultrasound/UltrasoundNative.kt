package org.kgajjar.mobileai.ultrasound

import android.graphics.Bitmap

/**
 * JNI Bridge to C++ Ultrasound SDK
 * Thread-safe wrapper for native ultrasound processing
 */
class UltrasoundNative {

    private var nativeHandle: Long = 0

    init {
        // Load native library
        System.loadLibrary("ultrasound_jni")
        nativeHandle = nativeInit()
    }

    /**
     * Process ultrasound frame from byte array
     * @param data Raw .ultra file bytes
     * @param normalize Apply normalization
     * @param denoise Apply denoising
     * @param gain Intensity gain (0.5 - 2.0)
     * @return OpenCV Mat address (use with matToBitmap)
     */
    fun processFrame(
        data: ByteArray,
        normalize: Boolean = true,
        denoise: Boolean = true,
        gain: Float = 1.0f
    ): Long {
        return nativeProcessFrame(nativeHandle, data, normalize, denoise, gain)
    }

    /**
     * Get frame metadata
     * @param data Raw .ultra file bytes
     * @return Pair of (frameNumber, timestamp in microseconds)
     */
    fun getFrameMetadata(data: ByteArray): FrameMetadata {
        val metadata = nativeGetFrameMetadata(nativeHandle, data)
        return FrameMetadata(
            frameNumber = metadata[0].toInt(),
            timestamp = metadata[1]
        )
    }

    /**
     * Convert OpenCV Mat to Android Bitmap
     * @param matAddress Mat address from processFrame
     * @param bitmap Target bitmap (must be mutable and correct size)
     */
    fun matToBitmap(matAddress: Long, bitmap: Bitmap) {
        nativeMatToBitmap(matAddress, bitmap)
    }

    /**
     * Release Mat memory
     * IMPORTANT: Always call this after you're done with a Mat
     */
    fun releaseMat(matAddress: Long) {
        nativeReleaseMat(matAddress)
    }

    /**
     * Get SDK version
     */
    fun getVersion(): String {
        return nativeGetVersion()
    }

    /**
     * Release native resources
     * Call this when done with the SDK
     */
    fun release() {
        if (nativeHandle != 0L) {
            nativeRelease(nativeHandle)
            nativeHandle = 0
        }
    }

    protected fun finalize() {
        release()
    }

    // Native methods
    private external fun nativeInit(): Long
    private external fun nativeRelease(handle: Long)
    private external fun nativeProcessFrame(
        handle: Long,
        data: ByteArray,
        normalize: Boolean,
        denoise: Boolean,
        gain: Float
    ): Long
    private external fun nativeGetFrameMetadata(handle: Long, data: ByteArray): LongArray
    private external fun nativeMatToBitmap(matAddress: Long, bitmap: Bitmap)
    private external fun nativeReleaseMat(matAddress: Long)
    private external fun nativeGetVersion(): String
}

/**
 * Frame metadata from .ultra file
 */
data class FrameMetadata(
    val frameNumber: Int,
    val timestamp: Long // microseconds
)
