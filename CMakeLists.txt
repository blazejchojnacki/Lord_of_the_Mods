# author: blazejchojnacki
# project: Lord_of_the_Mods

cmake_minimum_required(VERSION 3.17)
project(Lord_of_the_Mods LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Python and Pybind11
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# Build the module
pybind11_add_module(lotm src/cpp_py_bridge.cpp)
target_include_directories(lotm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# This line is strictly required for scikit-build-core to install the module into your venv
install(TARGETS lotm DESTINATION .)
