cmake_minimum_required(VERSION 3.26)

project(gravi)

# C++
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-w)

find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(pxr REQUIRED HINTS "${CMAKE_SOURCE_DIR}/lib/usd" NO_DEFAULT_PATH)
list(APPEND CMAKE_MODULE_PATH "${PXR_CMAKE_DIR}/cmake")

add_subdirectory(gravi)