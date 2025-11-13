# Ultrasound Demo Project

Full end-to-end ultrasound simulation for Android using C++, OpenCV, JNI, and Kotlin Coroutines/Flow.

## 📋 Overview

This project demonstrates a complete ultrasound imaging pipeline:
- **Dummy ultrasound data** (.ultra format) with realistic speckle noise
- **C++ SDK** for frame processing with OpenCV
- **JNI Bridge** for Android integration
- **Kotlin Android App** with Coroutines and Flow
- **Prepared for AI/TensorFlow Lite** integration

## 🏗️ Project Structure

```
MobileAi/
├── dummy_data/              # Generated .ultra files
│   ├── generate_ultra_files.py
│   └── frame_*.ultra        # 15 ultrasound frames
├── ultrasound_core/         # C++ SDK
│   ├── include/
│   │   └── ultrasound_core.h
│   ├── src/
│   │   └── ultrasound_core.cpp
│   ├── jni/
│   │   ├── ultrasound_jni.h
│   │   └── ultrasound_jni.cpp
│   └── CMakeLists.txt
├── composeApp/              # Android Kotlin App
│   ├── src/androidMain/kotlin/org/kgajjar/mobileai/
│   │   ├── ultrasound/
│   │   │   ├── UltrasoundNative.kt
│   │   │   ├── UltrasoundRepository.kt
│   │   │   └── UltrasoundViewModel.kt
│   │   └── screens/
│   │       └── UltrasoundScreen.android.kt
│   └── CMakeLists.txt
└── docs/                    # Documentation
    ├── README.md
    ├── FILE_FORMAT.md
    ├── SDK_API.md
    └── SETUP.md
```

## 🔄 Pipeline Flow

```
.ultra files → C++ SDK → JNI Bridge → Kotlin Flow → OpenCV Mat → Android UI
                  ↓
            (Future: AI/TFLite)
```

### Detailed Flow

1. **Data Generation**: Python script generates `.ultra` files with realistic ultrasound imagery
2. **File Loading**: Kotlin reads `.ultra` bytes from storage
3. **C++ Processing**: JNI calls C++ SDK to parse and process frames
4. **Image Processing**: OpenCV applies filters (denoise, normalize, gain)
5. **Stream Management**: Kotlin Flow manages asynchronous frame updates
6. **Display**: OpenCV Mat converted to Android Bitmap and displayed in Compose UI
7. **Future**: TFLite inference can be inserted after step 4

## 🚀 Quick Start

### Prerequisites

- Android Studio Arctic Fox or later
- NDK 23.1.7779620 or later
- CMake 3.18.1 or later
- OpenCV 4.x for Android
- Python 3.8+ (for generating dummy data)

### Setup Instructions

See [SETUP.md](SETUP.md) for detailed build instructions.

### Running the Demo

1. Generate dummy data:
```bash
cd dummy_data
python3 generate_ultra_files.py
```

2. Copy `.ultra` files to Android device:
```bash
adb push dummy_data/*.ultra /sdcard/Android/data/org.kgajjar.mobileai/files/ultrasound_data/
```

3. Build and run the Android app:
```bash
./gradlew :composeApp:installDebug
```

4. Open the app and navigate to the "Ultrasound" tab
5. Press "START" to begin the live scan simulation

## 📱 Features

### Current Features

- ✅ Realistic ultrasound frame simulation
- ✅ C++ SDK with OpenCV processing
- ✅ Thread-safe JNI bridge
- ✅ Kotlin Coroutines and Flow for async streaming
- ✅ Real-time frame display (1 FPS)
- ✅ Frame metadata display (frame number, timestamp)
- ✅ Progress tracking
- ✅ Dark theme UI optimized for medical imaging

### Processing Options

- **Normalization**: Adjust intensity levels
- **Denoising**: Fast non-local means denoising
- **Gain**: Intensity amplification
- **Sharpening**: Edge enhancement
- **Contrast**: Adjustable contrast and brightness

### Future Enhancements

- 🔜 TensorFlow Lite integration for AI inference
- 🔜 Real-time anomaly detection
- 🔜 USB device integration
- 🔜 DICOM export
- 🔜 Multi-frame analysis
- 🔜 3D reconstruction

## 📚 Documentation

- [File Format Specification](FILE_FORMAT.md)
- [SDK API Documentation](SDK_API.md)
- [Setup & Build Guide](SETUP.md)

## 🔧 Technical Details

### .ultra File Format

Binary format optimized for streaming:
- **Header**: 32 bytes (magic, dimensions, metadata, CRC)
- **Data**: 128×128 uint8 grayscale pixels
- **Total**: 16,416 bytes per frame

### C++ SDK

- **Cross-platform**: Designed for Android, iOS, desktop
- **OpenCV-based**: Industry-standard image processing
- **Modular**: Easy to extend with AI/TFLite
- **Performance**: Optimized for real-time processing

### Android App

- **Kotlin Multiplatform**: Shared code across platforms
- **Compose UI**: Modern declarative UI
- **Coroutines/Flow**: Reactive data streaming
- **Architecture**: MVVM with Repository pattern

## 🧪 Testing

### Unit Tests
```bash
./gradlew :composeApp:testDebugUnitTest
```

### Integration Tests
```bash
./gradlew :composeApp:connectedDebugAndroidTest
```

## 📄 License

This is a demonstration project for educational purposes.

## 🤝 Contributing

This is a demo project, but suggestions and improvements are welcome!

## 📞 Contact

For questions or issues, please open a GitHub issue.

---

**Note**: This is a simulation/demo project. For production medical imaging applications, ensure compliance with relevant medical device regulations (FDA, CE, etc.).
