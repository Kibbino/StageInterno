cmake_minimum_required(VERSION 3.16)

project(ModelloRobusto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

# Gurobi
include_directories($ENV{GUROBI_HOME}/include)
link_directories($ENV{GUROBI_HOME}/lib)

add_executable(ModelloRobusto
    src/main.cpp
    src/optimization/Master.cpp
    src/optimization/Slave.cpp
    src/data/generate_dac_data.cpp
    src/data/DACCSVGenerator.cpp
    src/profiler/AlgorithmProfiler.cpp
)

target_link_libraries(ModelloRobusto
    gurobi_c++
    gurobi130
)