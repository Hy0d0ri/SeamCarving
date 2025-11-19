cmake_minimum_required(VERSION 3.10)
project(SeamCarving)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# IMPORTANT: Update this path to YOUR OpenCV location
# Windows example: set(OpenCV_DIR "C:/opencv/build")
# Mac example: set(OpenCV_DIR "/usr/local/opt/opencv")
# Linux: Usually auto-detected, but you can set it if needed
set(OpenCV_DIR "D:/School/Year 3 Tri 1/Algorithm Analysis/Assignment 2/SeamCarving/opencv/build" CACHE PATH "OpenCV directory")

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Print OpenCV info (helpful for debugging)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Create executable from source files
add_executable(SeamCarving
    src/main.cpp
 "src/SeamCarver.cpp")

# Link OpenCV libraries
target_link_libraries(SeamCarving ${OpenCV_LIBS})