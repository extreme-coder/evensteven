cmake_minimum_required(VERSION 3.16)
project(evensteven_cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# KissFFT library (C)
add_library(kissfft STATIC
    third_party/kissfft/kiss_fft.c
    third_party/kissfft/kiss_fftr.c
)
target_include_directories(kissfft PUBLIC third_party/kissfft)

# Core static library
add_library(evensteven_core STATIC
    src/config.cpp
    src/logging.cpp
    src/audio_io.cpp
    src/preprocessing.cpp
    src/features.cpp
    src/loudness.cpp
    src/balance.cpp
    src/segmentation.cpp
    src/scoring.cpp
    src/export.cpp
)
target_include_directories(evensteven_core PUBLIC
    include
    third_party
    third_party/dr_libs
    third_party/json
)
target_link_libraries(evensteven_core PUBLIC kissfft)

# CLI executable
add_executable(evensteven src/main.cpp)
target_link_libraries(evensteven PRIVATE evensteven_core)

# Install
install(TARGETS evensteven RUNTIME DESTINATION bin)

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
