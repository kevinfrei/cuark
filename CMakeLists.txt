# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.28)

include(.cuark/EarlyConfig.cmake)

# Name of the project
project(cuark)

# Compiler setup, OS-specific configuration, etc...
include(.cuark/TheBasics.cmake)

# Find Conan2 generated libraries
include(.cuark/ConanLibImports.cmake)

# Stuff that doesn't play nice with Conan
include(.cuark/Extras.cmake)

# The actual C++ project lives in the cpp/ subdirectory
add_subdirectory(src)
