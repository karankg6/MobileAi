cmake_minimum_required(VERSION 3.18.1)
project(ultrasound_demo)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Add ultrasound_core include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../ultrasound_core/include
    ${CMAKE_SOURCE_DIR}/../ultrasound_core/jni
)

# Find OpenCV
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/src/androidMain/jniLibs/${ANDROID_ABI}/sdk/native/jni)
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Source files
set(ULTRASOUND_CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/../ultrasound_core/src/ultrasound_core.cpp
)

set(JNI_SOURCES
    ${CMAKE_SOURCE_DIR}/../ultrasound_core/jni/ultrasound_jni.cpp
)

# Create shared library
add_library(ultrasound_jni SHARED
    ${ULTRASOUND_CORE_SOURCES}
    ${JNI_SOURCES}
)

# Link libraries
target_link_libraries(ultrasound_jni
    ${OpenCV_LIBS}
    android
    log
    z
    jnigraphics
)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "==========================================")
message(STATUS "Ultrasound JNI Build Configuration")
message(STATUS "==========================================")
message(STATUS "OpenCV Version: ${OpenCV_VERSION}")
message(STATUS "OpenCV Libraries: ${OpenCV_LIBS}")
message(STATUS "Android ABI: ${ANDROID_ABI}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "==========================================")
