#!/bin/bash

# Instead of trying to write the USD Schema classes and headers, it is much easier to use the usdGenSchema command provided by USD.
# genSchema.sh automatically generates the required C++ code needed for Gravi.

# Get the directory where this script is located
USD_DIR=./lib/usd

# Set the correct USD paths.
export PXR_PLUGINPATH_NAME=$USD_DIR/plugin/usd
export PYTHONPATH=$USD_DIR/lib/python

# RUn the usdGenSchema command.
$USD_DIR/bin/usdGenSchema gravi/usdGravi/schema.usda gravi/usdGravi