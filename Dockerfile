FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    freeglut3-dev \
    libjsoncpp-dev \
    libpng-dev \
    libeigen3-dev \
    libboost-all-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create working directory
WORKDIR /app

# Copy the ARCSim-CPU source code
COPY . .

# Build the project
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make -j$(nproc)

# Set the working directory to the build directory
WORKDIR /app/build

# Default command to show help
ENTRYPOINT ["/app/build/arcsim"]
CMD ["--help"]

# Usage examples:
# Run with help:
#   docker run arcsim-cpu
# 
# Run a simulation:
#   docker run -v $(pwd)/output:/app/output arcsim-cpu simulate_offline ../conf/sphere.json output/simulation 