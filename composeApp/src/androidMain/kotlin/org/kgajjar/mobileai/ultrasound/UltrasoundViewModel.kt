package org.kgajjar.mobileai.ultrasound

import android.content.Context
import android.graphics.Bitmap
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.launch

/**
 * ViewModel for Ultrasound Screen
 * Manages ultrasound frame streaming and UI state
 */
class UltrasoundViewModel(context: Context) : ViewModel() {

    private val repository = UltrasoundRepository(context)

    private val _uiState = MutableStateFlow<UltrasoundUiState>(UltrasoundUiState.Idle)
    val uiState: StateFlow<UltrasoundUiState> = _uiState.asStateFlow()

    private var streamJob: Job? = null

    init {
        // Get available files on initialization
        val files = repository.getUltraFiles()
        if (files.isEmpty()) {
            _uiState.value = UltrasoundUiState.Error("No .ultra files found. Please copy files to ultrasound_data folder.")
        }
    }

    /**
     * Start streaming ultrasound frames
     */
    fun startScanning() {
        val files = repository.getUltraFiles()
        if (files.isEmpty()) {
            _uiState.value = UltrasoundUiState.Error("No .ultra files found")
            return
        }

        streamJob?.cancel()
        _uiState.value = UltrasoundUiState.Scanning(null, 0, files.size)

        streamJob = viewModelScope.launch {
            repository.streamFrames(
                frameFiles = files,
                delayMs = 1000L, // 1 second per frame
                normalize = true,
                denoise = true,
                gain = 1.2f
            )
                .catch { e ->
                    _uiState.value = UltrasoundUiState.Error(e.message ?: "Unknown error")
                }
                .collect { frame ->
                    _uiState.value = UltrasoundUiState.Scanning(
                        currentFrame = frame,
                        currentIndex = frame.frameNumber,
                        totalFrames = files.size
                    )
                }

            // When stream completes
            _uiState.value = UltrasoundUiState.ScanComplete
        }
    }

    /**
     * Stop scanning
     */
    fun stopScanning() {
        streamJob?.cancel()
        streamJob = null
        _uiState.value = UltrasoundUiState.Idle
    }

    /**
     * Reset to idle state
     */
    fun reset() {
        stopScanning()
    }

    /**
     * Get SDK version
     */
    fun getVersion(): String {
        return repository.getVersion()
    }

    override fun onCleared() {
        super.onCleared()
        stopScanning()
        repository.release()
    }
}

/**
 * UI State for Ultrasound Screen
 */
sealed class UltrasoundUiState {
    /**
     * Idle state - ready to start
     */
    object Idle : UltrasoundUiState()

    /**
     * Scanning in progress
     */
    data class Scanning(
        val currentFrame: UltrasoundFrame?,
        val currentIndex: Int,
        val totalFrames: Int
    ) : UltrasoundUiState()

    /**
     * Scan completed
     */
    object ScanComplete : UltrasoundUiState()

    /**
     * Error state
     */
    data class Error(val message: String) : UltrasoundUiState()
}
