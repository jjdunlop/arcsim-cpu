# ARCSim-CPU 🧵

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This repository contains **ARCSim-CPU**, a modified fork of the ARCSim-0.3.1 cloth simulation software, focusing on CPU-based simulation and incorporating several enhancements for ease of use, integration, and execution in HPC environments.

## Overview ✨

ARCSim is a powerful research tool for simulating the complex behaviour of cloth, including effects like folding, crumpling, tearing, and collisions. This fork maintains the core physics engine while adding improvements such as:

*   CMake-based build system
*   Integration with the Eigen library
*   Enhanced offline simulation mode with direct OBJ export and structured output
*   Robustness features for HPC environments
*   Containerisation support via Docker

## Key Features 🚀

*   **Modern Build System:** Uses CMake for easier cross-platform building.
*   **Direct OBJ Export:** Generates `.obj` files directly during offline simulation runs.
*   **Consistent Naming:** Saves output files with clear, predictable names (e.g., `cloth0_frame123.obj`).
*   **Automatic Directory Creation:** Output directories specified in commands are created automatically.
*   **Optional Obstacle Export:** Control whether obstacle geometry is saved using a command-line flag.
*   **Structured JSON Output:** Provides machine-readable JSON output for easier scripting and integration.
*   **Robust File Saving:** Includes retry logic for saving OBJ files, improving reliability on network filesystems (FSx, NFS).
*   **Headless Build Option:** Supports compiling without OpenGL dependencies for server/HPC use.
*   **Automated Installation:** Includes an `install.sh` script for Ubuntu dependencies.
*   **Docker Support:** Provides `Dockerfile` and `docker-compose.yml` for containerised deployment.

## Citation Requirements 📜

If you use this code or the original ARCSim library as part of work for a publication, **you must cite the relevant original papers**:

1.  **Remeshing:**
    > Rahul Narain, Armin Samii, and James F. O'Brien. "Adaptive Anisotropic Remeshing for Cloth Simulation". *ACM Transactions on Graphics*, 31(6):147:1–10, November 2012. Proceedings of ACM SIGGRAPH Asia 2012, Singapore.

2.  **Adaptive Sheets:**
    > Rahul Narain, Tobias Pfaff, and James F. O'Brien. "Folding and Crumpling Adaptive Sheets". *ACM Transactions on Graphics*, 32(4):51:1–8, July 2013. Proceedings of ACM SIGGRAPH 2013, Anaheim.

3.  **Tearing & Cracking:**
    > Tobias Pfaff, Rahul Narain, Juan Miguel de Joya, and James F. O'Brien. "Adaptive Tearing and Cracking of Thin Sheets". *ACM Transactions on Graphics*, 33(4):110:1–9, July 2014. Proceedings of ACM SIGGRAPH 2014, Vancouver.

## Installation 🛠️

### Option 1: Automated Installation (Ubuntu)

The simplest method for Ubuntu users:

```bash
# Clone the repository
git clone https://github.com/jjdunlop/arcsim-cpu.git
cd arcsim-cpu

# Run the installation script (may require sudo)
./install.sh
```
This script installs dependencies, builds the software, and places the executable in `./build/arcsim`.

### Option 2: Docker / Docker Compose

Build and run in a container, avoiding local dependency installation:

```bash
# Clone the repository
git clone https://github.com/jjdunlop/arcsim-cpu.git
cd arcsim-cpu

# Build the Docker image
docker build -t arcsim-cpu .

# Example: Run a simulation, mounting output directory
# (Replace paths and config file as needed)
docker run --rm -v "$(pwd)/output_docker:/app/output" arcsim-cpu \
  simulate_offline conf/sphere.json output/simulation

# --- OR using Docker Compose ---

# Build and run with docker-compose
docker-compose build
docker-compose run --rm arcsim-cpu \
  simulate_offline conf/sphere.json output/simulation
```

### Option 3: Manual Build (Other Linux / macOS)

1.  **Install Dependencies:** Ensure you have CMake, a C++14 compliant compiler (GCC/Clang), Make, and the libraries listed under [Dependencies](#dependencies).
2.  **Build:**
    ```bash
    # Clone the repository
    git clone https://github.com/jjdunlop/arcsim-cpu.git
    cd arcsim-cpu

    # Create build directory and run CMake
    mkdir build && cd build
    cmake ..

    # Compile (use -jN for parallel build, e.g., make -j$(nproc))
    make -j$(nproc)
    ```
    The executable will be at `./build/arcsim`.

## Usage ⚡

The primary way to run simulations without a GUI is using the `simulate_offline` command.

**Basic Syntax:**

```bash
./build/arcsim simulate_offline <config_file.json> [<output_directory>]
```

*   `<config_file.json>`: Path to the JSON file describing the simulation scene and parameters. (Examples in `conf/`)
*   `<output_directory>` (Optional): Path where simulation outputs (OBJ files, timing data, etc.) will be saved. If omitted, no files are saved.

**Command-Line Flags:**

*   `--no-export-obstacles`: Add this flag to prevent saving obstacle geometry (`obstacle*.obj`) files. Only cloth files (`cloth*.obj`) will be saved.
*   `--raw-output`: Use a simpler, human-readable text format for console output instead of the default structured JSON.

**Examples:**

```bash
# Run simulation defined in sphere.json, save results to output/sim_sphere
./build/arcsim simulate_offline conf/sphere.json output/sim_sphere

# Run simulation, save results, but don't save obstacle files
./build/arcsim simulate_offline --no-export-obstacles conf/sphere.json output/sim_sphere_nobs

# Run simulation with simple text output (useful for direct viewing)
./build/arcsim simulate_offline --raw-output conf/sphere.json output/sim_sphere_raw
```

## Output Format 📄

By default, `arcsim` prints structured JSON logs to standard output, useful for programmatic parsing:

```json
{ "status": "CONFIG", "details": "json_file=conf/sphere.json, output_dir=output/simulation, export_obstacles=true" }
{ "status": "INIT_PHYSICS_START", "details": "conf/sphere.json" }
{ "status": "SIMULATION_START", "details": "end_time=10, end_frame=250" }
{ "frame": 0, "step": 0, "time": 0.000000 }
{ "status": "SAVE_START", "details": "frame=0" }
{ "saved": "cloth0", "frame": 0, "file": "output/simulation/cloth0_frame0.obj" }
{ "status": "SAVE_COMPLETE", "details": "frame=0" }
```

Use the `--raw-output` flag for simpler, human-readable console output.

## Performance & Build Notes ⚙️

*   **Eigen Sparse Matrix Construction (Commit `6bedc86`)**:
    *   The assembly of the sparse system matrix in the physics step (`src/eigen.cpp`) was optimised using Eigen's `setFromTriplets` method. This significantly reduced the time spent in the dominant `Physics` step compared to inserting coefficients individually.
    *   *Safety:* This changes only the *method* of matrix construction, not the mathematical values, preserving simulation results.

*   **Link-Time Optimisation (LTO)**:
    *   LTO (`-flto`) was tested (Commit `d02b30f`) but subsequently disabled (Commit `4ba4c12`) due to linker errors when combined with the `NO_OPENGL` build flag.
    *   It remains disabled currently to ensure compatibility with headless builds.

*   **Robustness Enhancements (Commit `4ba4c12`)**:
    *   **Resilient OBJ Saving**: `save_obj` (`src/io.cpp`) now retries file operations for up to 10 seconds, improving robustness against transient filesystem errors in HPC/network storage environments.

*   **Headless Build (`NO_OPENGL`) (Commit `4ba4c12`)**:
    *   The build system supports the `NO_OPENGL` flag (`-DNO_OPENGL` added to CMake flags).
    *   This excludes all OpenGL/GLUT dependencies, producing a smaller executable suitable for servers/HPC and preventing potential graphics-related errors or runtime checks. Necessary code references to display/debug functions have been guarded (`#ifndef NO_OPENGL`).

*   **Threading (OpenMP)**:
    *   The code utilises OpenMP for parallelism, primarily in collision detection.
    *   By default, it may try to use all available cores. For HPC environments (e.g., Slurm), control thread count using the `OMP_NUM_THREADS` environment variable. Set it equal to the value requested via `--cpus-per-task` in your Slurm script for optimal performance and resource utilisation.
    *   Profiling suggests single-core execution (`OMP_NUM_THREADS=1`) might be most efficient for maximising simulation throughput on multi-core nodes due to serial bottlenecks in the physics step.

## Dependencies 📚

*   CMake (>= 3.20 recommended)
*   C++14 compliant compiler (GCC or Clang recommended)
*   Make (or other build generator like Ninja)
*   FreeGLUT (`freeglut3-dev` on Ubuntu) - *Only required if building **without** the `NO_OPENGL` flag.*
*   jsoncpp (`libjsoncpp-dev` on Ubuntu)
*   libPNG (`libpng-dev` on Ubuntu)
*   Eigen3 (`libeigen3-dev` on Ubuntu)
*   Boost (`libboost-all-dev` on Ubuntu - specifically system, filesystem, thread)
*   OpenMP (usually included with modern compilers)
*   ALGLIB (included in the `lib/` directory)

## Acknowledgements 🙏

This project is a fork and enhancement of ARCSim-0.3.1.

*   **Original ARCSim Developers:** Rahul Narain, Armin Samii, Tobias Pfaff, James F. O'Brien (UC Berkeley).
*   Based on improvements from the fork by [Wajov](https://github.com/Wajov/arcsim-0.3.1).
*   Further modified and maintained by [Jonathan Dunlop](https://github.com/jjdunlop/arcsim-cpu).