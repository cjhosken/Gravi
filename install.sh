#!/bin/bash

# Create the build directory
mkdir -p build
cd build

# Configure the CMakeLists and set the install location
cmake .. \
  -DCMAKE_INSTALL_PREFIX=../release/Gravi

# Build and install Gravi
cmake --build .
cmake --install .