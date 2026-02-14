#!/bin/bash

# FalconsDeck Build Script
# Simple helper script to build the application

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}FalconsDeck Build Script${NC}"
echo "======================================"

# Check if Qt is available
if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    echo -e "${RED}Error: Qt not found in PATH${NC}"
    echo "Please install Qt 6.11 or later and ensure it's in your PATH"
    echo "Or set CMAKE_PREFIX_PATH to your Qt installation"
    exit 1
fi

# Check CMake version
CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
echo -e "${YELLOW}CMake version: ${CMAKE_VERSION}${NC}"

# Create build directory
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. "$@"

# Build
echo -e "${YELLOW}Building...${NC}"
cmake --build . -j$(nproc)

# Success
echo ""
echo -e "${GREEN}✓ Build successful!${NC}"
echo ""
echo "To run the application:"
echo "  cd $BUILD_DIR && ./FalconsDeck"
echo ""
