#!/bin/bash

# Dodge the Creeps - Build and Run Script
# This script builds the C++ ECS game and runs it

set -e  # Exit on any error

echo "🎮 Dodge the Creeps - C++ ECS Version"
echo "======================================"

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
    echo "🚀 Starting game..."
    echo "Press ESC or close window to quit"
    echo "Use SPACE to start/restart game"
    echo "Use arrow keys to move"
    echo ""
    
    # Go back to cpp_version directory to run (so it can find entities.json and assets)
    cd ..
    ./build/DodgeTheCreeps
else
    echo "❌ Build failed!"
    exit 1
fi
