package org.kgajjar.mobileai.screens

import android.graphics.Bitmap
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import org.kgajjar.mobileai.ultrasound.*
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

/**
 * Ultrasound Demo Screen
 * Displays live ultrasound feed simulation
 */
@Composable
actual fun UltrasoundScreen() {
    val context = LocalContext.current
    val viewModel = remember { UltrasoundViewModel(context) }
    val uiState by viewModel.uiState.collectAsState()

    UltrasoundScreenContent(
        uiState = uiState,
        onStartScanning = { viewModel.startScanning() },
        onStopScanning = { viewModel.stopScanning() },
        onReset = { viewModel.reset() }
    )
}

@Composable
private fun UltrasoundScreenContent(
    uiState: UltrasoundUiState,
    onStartScanning: () -> Unit,
    onStopScanning: () -> Unit,
    onReset: () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        // Header
        Text(
            text = "Ultrasound Demo",
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold,
            color = Color.White,
            modifier = Modifier.padding(vertical = 16.dp)
        )

        // Ultrasound display area
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .aspectRatio(1f)
                .padding(8.dp),
            colors = CardDefaults.cardColors(
                containerColor = Color(0xFF1A1A1A)
            )
        ) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                when (uiState) {
                    is UltrasoundUiState.Idle -> {
                        Text(
                            text = "Ready to scan",
                            color = Color.Gray,
                            fontSize = 18.sp
                        )
                    }

                    is UltrasoundUiState.Scanning -> {
                        uiState.currentFrame?.let { frame ->
                            Image(
                                bitmap = frame.bitmap.asImageBitmap(),
                                contentDescription = "Ultrasound frame ${frame.frameNumber}",
                                modifier = Modifier.fillMaxSize(),
                                contentScale = ContentScale.Fit
                            )
                        } ?: run {
                            CircularProgressIndicator(
                                color = Color.White,
                                modifier = Modifier.size(48.dp)
                            )
                        }
                    }

                    is UltrasoundUiState.ScanComplete -> {
                        Text(
                            text = "Scan Complete",
                            color = Color.Green,
                            fontSize = 18.sp,
                            fontWeight = FontWeight.Bold
                        )
                    }

                    is UltrasoundUiState.Error -> {
                        Text(
                            text = uiState.message,
                            color = Color.Red,
                            fontSize = 14.sp,
                            modifier = Modifier.padding(16.dp)
                        )
                    }
                }
            }
        }

        Spacer(modifier = Modifier.height(16.dp))

        // Frame info
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(
                containerColor = Color(0xFF2A2A2A)
            )
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                when (uiState) {
                    is UltrasoundUiState.Scanning -> {
                        uiState.currentFrame?.let { frame ->
                            FrameInfoRow("Frame:", frame.frameNumber.toString())
                            FrameInfoRow("Timestamp:", frame.getFormattedTimestamp())
                            FrameInfoRow("File:", frame.filename)
                            FrameInfoRow(
                                "Progress:",
                                "${uiState.currentIndex + 1} / ${uiState.totalFrames}"
                            )

                            // Progress bar
                            Spacer(modifier = Modifier.height(8.dp))
                            LinearProgressIndicator(
                                progress = (uiState.currentIndex + 1).toFloat() / uiState.totalFrames.toFloat(),
                                modifier = Modifier.fillMaxWidth(),
                                color = Color(0xFF00BCD4)
                            )
                        }
                    }

                    is UltrasoundUiState.Idle -> {
                        Text(
                            text = "Press START to begin scanning",
                            color = Color.Gray,
                            fontSize = 14.sp
                        )
                    }

                    is UltrasoundUiState.ScanComplete -> {
                        Text(
                            text = "All frames processed successfully",
                            color = Color.Green,
                            fontSize = 14.sp
                        )
                    }

                    is UltrasoundUiState.Error -> {
                        Text(
                            text = "Error: ${uiState.message}",
                            color = Color.Red,
                            fontSize = 14.sp
                        )
                    }
                }
            }
        }

        Spacer(modifier = Modifier.height(16.dp))

        // Control buttons
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceEvenly
        ) {
            // Start/Stop button
            Button(
                onClick = {
                    when (uiState) {
                        is UltrasoundUiState.Scanning -> onStopScanning()
                        else -> onStartScanning()
                    }
                },
                colors = ButtonDefaults.buttonColors(
                    containerColor = if (uiState is UltrasoundUiState.Scanning)
                        Color(0xFFFF5252) else Color(0xFF00BCD4)
                ),
                modifier = Modifier.weight(1f).padding(horizontal = 4.dp)
            ) {
                Text(
                    text = if (uiState is UltrasoundUiState.Scanning) "STOP" else "START",
                    fontWeight = FontWeight.Bold
                )
            }

            // Reset button
            Button(
                onClick = onReset,
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color(0xFF757575)
                ),
                modifier = Modifier.weight(1f).padding(horizontal = 4.dp),
                enabled = uiState !is UltrasoundUiState.Idle
            ) {
                Text(
                    text = "RESET",
                    fontWeight = FontWeight.Bold
                )
            }
        }

        Spacer(modifier = Modifier.height(16.dp))

        // Info panel
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(
                containerColor = Color(0xFF2A2A2A)
            )
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(12.dp)
            ) {
                Text(
                    text = "Pipeline:",
                    color = Color(0xFF00BCD4),
                    fontSize = 12.sp,
                    fontWeight = FontWeight.Bold
                )
                Spacer(modifier = Modifier.height(4.dp))
                Text(
                    text = ".ultra files → C++ SDK → JNI → Kotlin Flow → OpenCV → UI",
                    color = Color.Gray,
                    fontSize = 10.sp,
                    fontFamily = FontFamily.Monospace
                )
            }
        }
    }
}

@Composable
private fun FrameInfoRow(label: String, value: String) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(
            text = label,
            color = Color.Gray,
            fontSize = 14.sp
        )
        Text(
            text = value,
            color = Color.White,
            fontSize = 14.sp,
            fontFamily = FontFamily.Monospace
        )
    }
    Spacer(modifier = Modifier.height(4.dp))
}
