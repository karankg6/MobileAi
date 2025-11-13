package org.kgajjar.mobileai.ultrasound

import android.content.Context
import android.graphics.Bitmap
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import java.io.File

/**
 * Repository for managing ultrasound frame data
 * Provides Flow-based streaming of ultrasound frames
 */
class UltrasoundRepository(private val context: Context) {

    private val nativeSDK = UltrasoundNative()

    /**
     * Stream ultrasound frames from files
     * @param frameFiles List of .ultra files to stream
     * @param delayMs Delay between frames (simulates real-time scanning)
     * @return Flow of UltrasoundFrame
     */
    fun streamFrames(
        frameFiles: List<File>,
        delayMs: Long = 1000L,
        normalize: Boolean = true,
        denoise: Boolean = true,
        gain: Float = 1.0f
    ): Flow<UltrasoundFrame> = flow {
        for (file in frameFiles) {
            try {
                // Read file bytes
                val frameData = file.readBytes()

                // Get metadata
                val metadata = nativeSDK.getFrameMetadata(frameData)

                // Process frame
                val matAddress = nativeSDK.processFrame(frameData, normalize, denoise, gain)

                // Create bitmap
                val bitmap = Bitmap.createBitmap(128, 128, Bitmap.Config.ARGB_8888)
                nativeSDK.matToBitmap(matAddress, bitmap)

                // Release native Mat memory
                nativeSDK.releaseMat(matAddress)

                // Emit frame
                emit(
                    UltrasoundFrame(
                        bitmap = bitmap,
                        frameNumber = metadata.frameNumber,
                        timestamp = metadata.timestamp,
                        filename = file.name
                    )
                )

                // Delay to simulate real-time scanning
                delay(delayMs)

            } catch (e: Exception) {
                // Log error and continue with next frame
                e.printStackTrace()
            }
        }
    }.flowOn(Dispatchers.IO)

    /**
     * Load a single frame
     */
    suspend fun loadSingleFrame(file: File): UltrasoundFrame? {
        return try {
            val frameData = file.readBytes()
            val metadata = nativeSDK.getFrameMetadata(frameData)
            val matAddress = nativeSDK.processFrame(frameData)

            val bitmap = Bitmap.createBitmap(128, 128, Bitmap.Config.ARGB_8888)
            nativeSDK.matToBitmap(matAddress, bitmap)
            nativeSDK.releaseMat(matAddress)

            UltrasoundFrame(
                bitmap = bitmap,
                frameNumber = metadata.frameNumber,
                timestamp = metadata.timestamp,
                filename = file.name
            )
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    /**
     * Get list of .ultra files from assets or external storage
     */
    fun getUltraFiles(): List<File> {
        // This assumes .ultra files are in external storage or assets
        // For demo purposes, you can copy files from dummy_data/ to device

        val ultraDir = File(context.getExternalFilesDir(null), "ultrasound_data")
        if (!ultraDir.exists()) {
            ultraDir.mkdirs()
        }

        val files = ultraDir.listFiles { file ->
            file.extension == "ultra"
        }?.toList()?.sortedBy { it.name } ?: emptyList()

        return files
    }

    /**
     * Get SDK version
     */
    fun getVersion(): String {
        return nativeSDK.getVersion()
    }

    /**
     * Release resources
     */
    fun release() {
        nativeSDK.release()
    }
}

/**
 * Ultrasound frame data
 */
data class UltrasoundFrame(
    val bitmap: Bitmap,
    val frameNumber: Int,
    val timestamp: Long,
    val filename: String
) {
    /**
     * Get formatted timestamp
     */
    fun getFormattedTimestamp(): String {
        val seconds = timestamp / 1_000_000.0
        return String.format("%.3f s", seconds)
    }
}
