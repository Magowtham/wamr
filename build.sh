#!/bin/bash


set -e 



ROOT_DIR=$(pwd)
WAMR_DIR="$ROOT_DIR/wamr/wamr_soruce"
BUILD_DIR="$ROOT_DIR/build"

mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

cmake ..

# Run CMake
make -j$(nproc)


