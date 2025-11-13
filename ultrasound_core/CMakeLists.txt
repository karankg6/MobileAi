cmake_minimum_required(VERSION 3.10)
project(ultrasound_core VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find zlib for CRC32
find_package(ZLIB REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/ultrasound_core.cpp
)

# Header files
set(HEADERS
    include/ultrasound_core.h
)

# Create static library
add_library(ultrasound_core STATIC ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(ultrasound_core
    ${OpenCV_LIBS}
    ${ZLIB_LIBRARIES}
)

# Set include directories for the library
target_include_directories(ultrasound_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Installation rules
install(TARGETS ultrasound_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include
)

# Print configuration
message(STATUS "Ultrasound Core SDK Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  OpenCV version: ${OpenCV_VERSION}")
message(STATUS "  OpenCV libraries: ${OpenCV_LIBS}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
