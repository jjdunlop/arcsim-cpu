# ARCSim-CPU

This is a fork of the ARCSim-0.3.1 cloth simulation software. It includes several improvements over the original version:

1. Replaced makefile with CMake for easier building
2. Replaced some old libraries with Eigen
3. Enhanced the `simulate_offline` mode to directly save OBJ files using a consistent naming convention
4. Automatic creation of output directories
5. Added option to disable obstacle exports

## Modifications

The main modifications in this fork include:

- **Direct OBJ Export**: The simulation now directly generates OBJ files in addition to binary state files
- **Naming Convention**: Files are saved as `cloth{number}_frame{frame}.obj` and `obstacle{number}_frame{frame}.obj`
- **Automatic Directory Creation**: Output directories are created automatically if they don't exist
- **Optional Obstacle Export**: Use the `--no-export-obstacles` flag to skip exporting obstacle OBJs
- **Improved Command Naming**: The command is now available as both `simulateoffline` and `simulate_offline`

## Dependencies

* FreeGLUT
* jsoncpp
* libPNG
* Eigen3
* ALGLIB (already included in the lib directory)
* Boost

## Building

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Running a Simulation

```bash
# Standard simulation with all exports
./build/arcsim simulate_offline conf/sphere.json output/simulation

# Simulation without exporting obstacle OBJs
./build/arcsim simulate_offline --no-export-obstacles conf/sphere.json output/simulation
```

This will run the simulation and save both binary state files and OBJ files in the output directory.

## Credit

This is a modified version of ARCSim-0.3.1 by Jonathan Dunlop, based on the improved version by [Wajov](https://github.com/Wajov/arcsim-0.3.1).

The original ARCSim was developed by Rahul Narain, Armin Samii, and Tobias Pfaff.