cmake_minimum_required(VERSION 3.15)

project(cropac_binaural_head VERSION 1.0.0)

# Build everything as "Release" by default
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Configure CMake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Platform specific flags and definitions
if(MSVC)
    set(CMAKE_GENERATOR_PLATFORM "x64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -Wextra -Wno-inconsistent-missing-override")  
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
elseif(UNIX AND NOT APPLE AND NOT ANDROID)
    add_definitions(
        #-std=c++11
        -DLINUX=1   
    )
endif()

# Disable JUCE extras and examples:
option(JUCE_BUILD_EXTRAS   "Build JUCE Extras"   OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)

# Disable SAF tests, but enable SAF examples:
option(SAF_BUILD_TESTS    "Build SAF unit tests." OFF)
option(SAF_BUILD_EXAMPLES "Build SAF examples."   OFF)

# Add JUCE, Spatial_Audio_Framework, and VST2_SDK to the project
add_subdirectory(SDKs) 

# Add libs
add_subdirectory(libs)

# Configure plugin
option(BUILD_PLUGIN_FORMAT_VST2 "Build VST2 plugin" ON)
option(BUILD_PLUGIN_FORMAT_VST3 "Build VST3 plugin" OFF)
option(BUILD_PLUGIN_FORMAT_AU   "Build AU plugin"   OFF)
option(BUILD_PLUGIN_FORMAT_LV2  "Build LV2 plugin"  OFF)
option(BUILD_PLUGIN_FORMAT_AAX  "Build AAX plugins"  OFF)
option(BUILD_PLUGIN_FORMAT_STANDALONE  "Build standalone versions of the plugins"  OFF)
add_subdirectory(audio_plugin)