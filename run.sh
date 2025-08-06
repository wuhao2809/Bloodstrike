#!/bin/bash

# Bloodstrike 2D - Build and Run Script
# This script builds the C++ ECS shooting game and runs it

set -e  # Exit on any error

echo "🎮 Bloodstrike 2D - C++ ECS Shooter"
echo "===================================="

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "📁 Working directory: $SCRIPT_DIR"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "📂 Creating build directory..."
    mkdir -p build
fi

cd build

echo "🔨 Building project..."
echo "Running cmake..."
cmake ..

echo "Running make..."
make

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "🚀 Starting Bloodstrike 2D..."
    echo "Press ESC or close window to quit"
    echo "Use SPACE to start/restart game"
    echo "Use arrow keys to move"
    echo "Use mouse to aim and LEFT CLICK to shoot"
    echo ""
    
    # Go back to main directory to run (so it can find entities.json and assets)
    cd ..
    ./build/Bloodstrike
else
    echo "❌ Build failed!"
    exit 1
fi
