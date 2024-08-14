cmake_minimum_required(VERSION 3.10)

# Project name
project(Simulation)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directory for nlohmann json
include_directories(${CMAKE_SOURCE_DIR}/src/include/json/include)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Source files
set(SOURCE_FILES src/main.cpp src/Simulator.cpp)

# Executable
add_executable(simulation ${SOURCE_FILES})

# Link libraries
target_link_libraries(simulation ${SDL2_LIBRARIES} ${OpenCV_LIBS})


