cmake_minimum_required(VERSION 3.20)

# Project name and information
project(arcsim 
    VERSION 1.0.0
    DESCRIPTION "Enhanced ARCSim cloth simulation software"
    LANGUAGES CXX)

# Output helpful information at the start of the build
message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Building for ${CMAKE_SYSTEM_NAME}")

# Modern C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set optimization flags for cross-instance compatibility
# Use a baseline architecture that works on both Intel and AMD CPUs
message(STATUS "Setting up for cross-platform CPU compatibility")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Optimization level
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    
    # Target x86-64-v2 instruction set (SSE4.2) - compatible with all listed instances
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v2")
    
    # Additional optimizations that don't affect numerical results
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -finline-functions")
    
    # Explicitly avoid using native architecture
    message(STATUS "Using generic x86-64-v2 target for cross-instance compatibility")
endif()

# Set Windows-specific settings
if (WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    add_compile_definitions(_USE_MATH_DEFINES)
endif()

# Source files
include_directories(src)
file(GLOB_RECURSE SOURCE "src/*.cpp" "src/*.hpp")
add_executable(arcsim ${SOURCE})

# Function to check for required packages and show helpful error messages
function(check_required_package package_name)
    if(NOT ${${package_name}_FOUND})
        message(FATAL_ERROR "Required package ${package_name} not found. Please install it using:\n"
                "sudo apt-get install -y ${ARGN}")
    else()
        message(STATUS "Found ${package_name}: ${${package_name}_VERSION}")
    endif()
endfunction()

# OpenGL
find_package(OpenGL REQUIRED)
check_required_package(OpenGL "libgl1-mesa-dev")

# GLUT
if (WIN32)
    find_package(FreeGLUT REQUIRED)
else()
    find_package(GLUT REQUIRED)
    check_required_package(GLUT "freeglut3-dev")
endif()

# jsoncpp
if (WIN32)
    find_package(jsoncpp CONFIG REQUIRED)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JSONCPP jsoncpp)
    if(NOT JSONCPP_FOUND)
        message(FATAL_ERROR "jsoncpp not found. Please install it using:\n"
                "sudo apt-get install -y libjsoncpp-dev")
    else()
        message(STATUS "Found jsoncpp: ${JSONCPP_VERSION}")
    endif()
    include_directories(/usr/include/jsoncpp)
endif()

# PNG
find_package(PNG REQUIRED)
check_required_package(PNG "libpng-dev")

# Eigen
find_package(Eigen3 REQUIRED)
check_required_package(Eigen3 "libeigen3-dev")

# ALGLIB
add_subdirectory(lib/alglib)
message(STATUS "Using bundled ALGLIB library")

# Boost
find_package(Boost COMPONENTS system filesystem thread REQUIRED)
check_required_package(Boost "libboost-all-dev")

# OpenMP
find_package(OpenMP REQUIRED)
check_required_package(OpenMP "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

if (WIN32)
    target_link_libraries(
        arcsim
        ${OPENGL_LIBRARIES}
        FreeGLUT::freeglut
        JsonCpp::JsonCpp
        ${PNG_LIBRARIES}
        alglib
        ${Boost_LIBRARIES}
    )
else()
    target_link_libraries(
        arcsim
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARIES}
        jsoncpp
        ${PNG_LIBRARIES}
        alglib
        ${Boost_LIBRARIES}
    )
endif()

# Installation
install(TARGETS arcsim DESTINATION bin)

# Final status message
message(STATUS "${PROJECT_NAME} configuration complete. Type 'make' to build.")