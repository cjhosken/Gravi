#!/bin/bash

# Define the build and installation directories
BUILD_DIR=build             # Directory to store build files
INSTALL_DIR=lib/usd         # Directory where the USD libraries and components will be installed
USD_TAG="v25.02a"           # Define the specific USD tag version to be used

# For NCCA Labs where Cmake is 3.26.5
# USD needs 3.27 or later
CMAKE_BIN=$HOME/Documents/cmake-4.0.1-linux-x86_64/bin
if [ -d "$CMAKE_BIN" ]; then
    export PATH="$CMAKE_BIN:$PATH"
fi

# Create necessary directories (if they don't already exist) for building and installing
echo "Creating necessary directories..."
mkdir -p $BUILD_DIR         # Create the build directory
mkdir -p $INSTALL_DIR       # Create the installation directory

# Navigate to the build directory to begin the build process
cd $BUILD_DIR
echo "Navigating to the build directory: $BUILD_DIR"

# Clone the OpenUSD repository from GitHub into the 'USDPrefix' directory under the build directory
echo "Cloning the OpenUSD repository from GitHub..."
git clone https://github.com/PixarAnimationStudios/OpenUSD.git --branch $USD_TAG ./USD-prefix

# Display the USD version being used
echo "Displaying the USD version..."
echo "Using OpenUSD version: $USD_TAG"

# Display the Python version being used
echo "Displaying Python version..."
PYTHON_VERSION=$(python3 --version)
echo "Using Python version: $PYTHON_VERSION"

# Install necessary Python dependencies (PyOpenGL, PySide6, jinja2)
echo "Installing necessary Python dependencies (PyOpenGL, PySide6, jinja2)..."
uv pip install PyOpenGL PySide6 jinja2

# Return to the build directory to ensure we're in the correct location
cd $BUILD_DIR

unset CMAKE_TOOLCHAIN_FILE
python3 ./USD-prefix//build_scripts/build_usd.py --help
echo "Running the USD build script with specified configuration options..."
python3 ./USD-prefix/build_scripts/build_usd.py \
    --build-monolithic \
    --no-tests \
    --no-examples \
    --no-tutorials \
    --tools \
    --no-docs \
    --python \
    --openvdb \
    --no-embree \
    --no-prman \
    --openimageio \
    --no-opencolorio \
    --no-onetbb \
    --ptex \
    --no-materialx \
    --alembic \
    --no-mayapy-tests \
    --no-animx-tests \
    --cmake-build-args="-DCMAKE_POLICY_VERSION_MINIMUM=3.5" \
    ../${INSTALL_DIR}

# Final message indicating completion
echo "USD build process completed successfully! The libraries are installed in $INSTALL_DIR."

export PYTHONPATH="$INSTALL_DIR/lib/python:$PYTHONPATH"
export PATH="$INSTALL_DIR/bin:$PATH"
