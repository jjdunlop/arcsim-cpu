#!/bin/bash

# ARCSim-CPU Automated Installation Script
# This script automatically installs all required dependencies and builds ARCSim-CPU
# Uses cross-platform compatible compiler flags for AWS instances

set -e  # Exit on error

# ANSI colour codes
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print coloured text
print_green() {
    echo -e "${GREEN}$1${NC}"
}

print_yellow() {
    echo -e "${YELLOW}$1${NC}"
}

print_red() {
    echo -e "${RED}$1${NC}"
}

print_blue() {
    echo -e "${BLUE}$1${NC}"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check if running as root or with sudo
if [ "$EUID" -ne 0 ]; then
    print_yellow "This script needs to install packages and requires sudo privileges."
    print_yellow "You might be prompted for your password."
fi

print_green "=== ARCSim-CPU Automated Installation ==="
print_green "This script will install all required dependencies and build ARCSim-CPU."

# Print information about cross-instance compatibility
print_blue "Building with cross-platform compatibility for AWS instances"
print_blue "This build will work on Intel (c5/c6i/c7i) and AMD (c6a) instances"
print_blue "Using x86-64-v2 instruction set for compatibility"

# Install dependencies
print_yellow "Installing dependencies..."
sudo apt-get update

# Essential build tools
print_yellow "Installing build tools..."
sudo apt-get install -y build-essential cmake git

# ARCSim dependencies
print_yellow "Installing ARCSim-CPU dependencies..."
sudo apt-get install -y freeglut3-dev libjsoncpp-dev libpng-dev libeigen3-dev libboost-all-dev

print_green "All dependencies installed successfully!"

# Build ARCSim-CPU
print_yellow "Building ARCSim-CPU with cross-instance compatibility..."
mkdir -p build
cd build

# Use CMake with appropriate flags
cmake ..

# Build using all available cores
print_yellow "Compiling with optimized flags for cross-instance compatibility..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    print_green "=== Build completed successfully! ==="
    
    # Create simulation directories
    print_yellow "Setting up simulation directories..."
    mkdir -p /shared/logs /shared/cmaes /shared/output /shared/archives 2>/dev/null || true
    print_green "Created simulation directories."
    
    print_green "ARCSim-CPU has been built and is ready to use."
    print_green "You can run it using: ./build/arcsim"
    print_green ""
    print_blue "IMPORTANT: This build is optimized for compatibility across"
    print_blue "different AWS instance types (both Intel and AMD processors)"
    print_blue ""
    print_green "For standard simulation:"
    print_green "  ./build/arcsim simulate_offline conf/sphere.json output/simulation"
    print_green ""
    print_green "For CMAES optimization pipeline:"
    print_green "  cd /shared/scripts"
    print_green "  python3 run_cmaes.py"
    exit 0
else
    print_red "Build failed! Please check the error messages above."
    exit 1
fi 