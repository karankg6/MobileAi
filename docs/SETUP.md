# Setup & Build Guide

Complete instructions for building and running the Ultrasound Demo project.

## Prerequisites

### Required Software

| Tool | Version | Purpose |
|------|---------|---------|
| Android Studio | Arctic Fox+ | IDE and build system |
| Android NDK | 23.1.7779620+ | Native C++ compilation |
| CMake | 3.18.1+ | C++ build system |
| Python | 3.8+ | Dummy data generation |
| Git | Any | Version control |

### Android Requirements

- **Minimum SDK**: API 24 (Android 7.0)
- **Target SDK**: API 34 (Android 14)
- **Test Device**: Physical device or emulator with ARM/ARM64

### OpenCV for Android

⚠️ **Important**: OpenCV must be manually downloaded and integrated.

#### Download OpenCV

1. Visit: https://opencv.org/releases/
2. Download: **OpenCV 4.8.0 Android SDK** (or latest 4.x)
3. Extract to a known location

#### Integrate OpenCV

**Option 1: Manual Integration (Recommended)**

```bash
# Extract OpenCV SDK
cd ~/Downloads
unzip opencv-4.8.0-android-sdk.zip

# Copy to project
cp -r OpenCV-android-sdk/sdk MobileAi/composeApp/src/androidMain/jniLibs/
```

**Option 2: Gradle Dependency**

Add to `composeApp/build.gradle.kts`:

```kotlin
dependencies {
    implementation("org.opencv:opencv-android:4.8.0")
}
```

Then sync and download OpenCV AAR from Maven Central.

---

## Step-by-Step Setup

### 1. Clone Repository

```bash
git clone https://github.com/karankg6/MobileAi.git
cd MobileAi
git checkout claude/ultrasound-android-demo-011CV4vFrjwWxBNATS99kKXJ
```

### 2. Install Dependencies

#### macOS

```bash
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install Python dependencies
pip3 install numpy

# Install Android Studio
brew install --cask android-studio
```

#### Linux (Ubuntu/Debian)

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install Python and dependencies
sudo apt install python3 python3-pip -y
pip3 install numpy

# Install Android Studio (or download from website)
sudo snap install android-studio --classic
```

#### Windows

1. Download Python 3.8+ from https://python.org
2. Install: `pip install numpy`
3. Download Android Studio from https://developer.android.com/studio

### 3. Configure Android Studio

#### Install SDK Components

1. Open Android Studio
2. Go to: **Tools → SDK Manager**
3. Install:
   - ✅ Android SDK Platform 34
   - ✅ Android SDK Build-Tools 34.0.0
   - ✅ NDK (Side by side) 23.1.7779620
   - ✅ CMake 3.18.1

#### Configure NDK Path

**Method 1: In `local.properties`**

```properties
sdk.dir=/Users/your_username/Library/Android/sdk
ndk.dir=/Users/your_username/Library/Android/sdk/ndk/23.1.7779620
```

**Method 2: Environment Variable**

```bash
export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/23.1.7779620
```

### 4. Setup OpenCV

#### Directory Structure

```
MobileAi/
└── composeApp/
    └── src/
        └── androidMain/
            └── jniLibs/
                ├── arm64-v8a/
                │   └── sdk/
                │       └── native/
                │           ├── jni/
                │           │   └── OpenCVConfig.cmake
                │           └── libs/
                │               └── libopencv_java4.so
                ├── armeabi-v7a/
                │   └── sdk/...
                ├── x86/
                │   └── sdk/...
                └── x86_64/
                    └── sdk/...
```

#### Copy OpenCV Files

```bash
cd MobileAi/composeApp/src/androidMain
mkdir -p jniLibs

# Copy for each ABI
for abi in arm64-v8a armeabi-v7a x86 x86_64; do
    mkdir -p jniLibs/$abi
    cp -r ~/Downloads/OpenCV-android-sdk/sdk jniLibs/$abi/
done
```

**Alternative**: Symlink to save space

```bash
ln -s ~/Downloads/OpenCV-android-sdk composeApp/src/androidMain/opencv-sdk
```

Then update CMakeLists.txt:
```cmake
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/src/androidMain/opencv-sdk/sdk/native/jni)
```

### 5. Generate Dummy Data

```bash
cd dummy_data
python3 generate_ultra_files.py
```

**Output**:
```
Generating 15 ultrasound files...
Generated: frame_0000.ultra (Frame 0, CRC: 0xB42CE5FB)
...
✓ Successfully generated 15 files
Total size: 240.47 KB
```

**Verify**:
```bash
ls -lh *.ultra
file frame_0000.ultra
```

### 6. Build Project

#### Command Line

```bash
# Clean build
./gradlew clean

# Build debug APK
./gradlew :composeApp:assembleDebug

# Build and install to device
./gradlew :composeApp:installDebug

# Build release APK (requires signing)
./gradlew :composeApp:assembleRelease
```

#### Android Studio

1. Open project: **File → Open → MobileAi/**
2. Wait for Gradle sync
3. Select device/emulator
4. Click **Run** (▶️)

### 7. Deploy Dummy Data to Device

#### Option 1: ADB Push

```bash
# Create directory
adb shell mkdir -p /sdcard/Android/data/org.kgajjar.mobileai/files/ultrasound_data

# Push files
adb push dummy_data/*.ultra /sdcard/Android/data/org.kgajjar.mobileai/files/ultrasound_data/

# Verify
adb shell ls /sdcard/Android/data/org.kgajjar.mobileai/files/ultrasound_data/
```

#### Option 2: Manual Copy

1. Connect device via USB
2. Enable file transfer mode
3. Copy `dummy_data/*.ultra` to:
   ```
   Internal Storage/Android/data/org.kgajjar.mobileai/files/ultrasound_data/
   ```

#### Option 3: Include in Assets (Build-time)

Add to `composeApp/src/androidMain/assets/`:

```bash
cp dummy_data/*.ultra composeApp/src/androidMain/assets/
```

Then load in code:
```kotlin
context.assets.open("frame_0000.ultra").readBytes()
```

---

## Build Configuration

### CMakeLists.txt

Located at: `MobileAi/composeApp/CMakeLists.txt`

Key sections:

```cmake
# OpenCV path (update if needed)
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/src/androidMain/jniLibs/${ANDROID_ABI}/sdk/native/jni)
find_package(OpenCV REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../ultrasound_core/include
    ${CMAKE_SOURCE_DIR}/../ultrasound_core/jni
    ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(ultrasound_jni
    ${OpenCV_LIBS}
    android
    log
    z
    jnigraphics
)
```

### build.gradle.kts

Located at: `MobileAi/composeApp/build.gradle.kts`

Key sections:

```kotlin
android {
    defaultConfig {
        // NDK configuration
        ndk {
            abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64"))
        }

        // CMake configuration
        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17", "-frtti", "-fexceptions")
                arguments("-DANDROID_STL=c++_shared")
            }
        }
    }

    // CMake path
    externalNativeBuild {
        cmake {
            path("CMakeLists.txt")
            version = "3.18.1"
        }
    }
}
```

---

## Running the App

### Launch

1. Install app: `./gradlew installDebug`
2. Open app on device
3. Navigate to **Ultrasound** tab (heart icon)
4. Press **START**

### Expected Behavior

- Frames display at 1 FPS
- Frame number increments: 0 → 14
- Timestamp updates in microseconds
- Progress bar fills
- "Scan Complete" message at end

### Troubleshooting

#### "No .ultra files found"

**Solution**: Copy files to device (see Step 7 above)

```bash
adb push dummy_data/*.ultra /sdcard/Android/data/org.kgajjar.mobileai/files/ultrasound_data/
```

#### "Library not found: libultrasound_jni.so"

**Cause**: CMake build failed or OpenCV not found

**Solution**:
1. Check CMakeLists.txt OpenCV path
2. Verify OpenCV files exist in jniLibs/
3. Clean and rebuild:
   ```bash
   ./gradlew clean
   ./gradlew :composeApp:assembleDebug
   ```

#### "java.lang.UnsatisfiedLinkError"

**Cause**: JNI method signature mismatch

**Solution**:
1. Verify package name in JNI: `org_kgajjar_mobileai_ultrasound_UltrasoundNative`
2. Check method names match exactly
3. Rebuild native library

#### OpenCV not found (CMake error)

**Error**:
```
Could NOT find OpenCV (missing: OpenCV_DIR)
```

**Solution**:
1. Download OpenCV Android SDK
2. Update `OpenCV_DIR` in CMakeLists.txt
3. Ensure path structure is correct

---

## Build for Release

### 1. Generate Signing Key

```bash
keytool -genkey -v -keystore ultrasound-demo.keystore \
    -alias ultrasound -keyalg RSA -keysize 2048 -validity 10000
```

### 2. Configure Signing

Create `keystore.properties`:

```properties
storeFile=ultrasound-demo.keystore
storePassword=YOUR_PASSWORD
keyAlias=ultrasound
keyPassword=YOUR_PASSWORD
```

### 3. Update build.gradle.kts

```kotlin
android {
    signingConfigs {
        create("release") {
            storeFile = file("ultrasound-demo.keystore")
            storePassword = "YOUR_PASSWORD"
            keyAlias = "ultrasound"
            keyPassword = "YOUR_PASSWORD"
        }
    }

    buildTypes {
        getByName("release") {
            isMinifyEnabled = true
            isShrinkResources = true
            signingConfig = signingConfigs.getByName("release")
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }
}
```

### 4. Build Release APK

```bash
./gradlew :composeApp:assembleRelease
```

**Output**: `composeApp/build/outputs/apk/release/composeApp-release.apk`

---

## Performance Optimization

### Release Build Flags

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG")
endif()
```

### ProGuard Rules

Create `composeApp/proguard-rules.pro`:

```proguard
# Keep native methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep UltrasoundNative
-keep class org.kgajjar.mobileai.ultrasound.UltrasoundNative { *; }

# OpenCV
-keep class org.opencv.** { *; }
```

### Reduce APK Size

1. Enable code shrinking
2. Use specific ABI:
   ```kotlin
   ndk {
       abiFilters.addAll(listOf("arm64-v8a"))  // 64-bit only
   }
   ```
3. Enable APK splitting:
   ```kotlin
   splits {
       abi {
           isEnable = true
           reset()
           include("arm64-v8a", "armeabi-v7a")
       }
   }
   ```

---

## Testing

### Unit Tests

```bash
./gradlew :composeApp:testDebugUnitTest
```

### Instrumented Tests

```bash
./gradlew :composeApp:connectedDebugAndroidTest
```

### Manual Testing Checklist

- [ ] App launches without crash
- [ ] Ultrasound tab displays
- [ ] START button begins scan
- [ ] Frames display at ~1 FPS
- [ ] Frame metadata updates
- [ ] Progress bar advances
- [ ] STOP button halts scan
- [ ] RESET button returns to idle
- [ ] No memory leaks after multiple scans

---

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Build Ultrasound Demo

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Set up JDK 11
        uses: actions/setup-java@v3
        with:
          java-version: '11'
          distribution: 'temurin'

      - name: Setup Android SDK
        uses: android-actions/setup-android@v2

      - name: Generate dummy data
        run: |
          cd dummy_data
          python3 generate_ultra_files.py

      - name: Build APK
        run: ./gradlew :composeApp:assembleDebug

      - name: Upload APK
        uses: actions/upload-artifact@v3
        with:
          name: app-debug
          path: composeApp/build/outputs/apk/debug/*.apk
```

---

## Common Issues

### Issue: "Execution failed for task ':composeApp:externalNativeBuildDebug'"

**Solution**: Update NDK path, clean build, verify CMakeLists.txt

### Issue: Out of Memory (OOM)

**Solution**: Increase Gradle memory:

`gradle.properties`:
```properties
org.gradle.jvmargs=-Xmx4096m -XX:MaxPermSize=1024m
```

### Issue: Slow Build Times

**Solutions**:
1. Enable parallel builds: `org.gradle.parallel=true`
2. Enable configuration cache: `org.gradle.configuration-cache=true`
3. Use build cache: `org.gradle.caching=true`

---

## Next Steps

1. ✅ Build and run the demo
2. 📚 Read [SDK_API.md](SDK_API.md) for API details
3. 🔬 Experiment with processing parameters
4. 🧠 Prepare for AI/TFLite integration
5. 📱 Test on multiple devices

---

## Support

For issues, see:
- [GitHub Issues](https://github.com/karankg6/MobileAi/issues)
- [FILE_FORMAT.md](FILE_FORMAT.md) for file format questions
- [SDK_API.md](SDK_API.md) for API usage

---

*Last updated: November 2025*
