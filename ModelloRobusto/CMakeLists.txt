cmake_minimum_required(VERSION 3.16)

project(ModelloRobusto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gurobi
include_directories($ENV{GUROBI_HOME}/include)
link_directories($ENV{GUROBI_HOME}/lib)

add_executable(ModelloRobusto
    main.cpp
    Master.cpp
    Slave.cpp
    generate_dac_data.cpp
    DACCSVGenerator.cpp
)

target_link_libraries(ModelloRobusto
    gurobi_c++
    gurobi130
)