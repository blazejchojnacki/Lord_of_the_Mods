# author: blazejchojnacki
# project: Lord_of_the_Mods

cmake_minimum_required(VERSION 3.17)
project(Lord_of_the_Mods LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Force CMake to find the exact Python interpreter currently running
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# 2. Bypass local installations and download pybind11 directly from GitHub
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.13.1 # Latest stable release known to support modern Python
)
FetchContent_MakeAvailable(pybind11)

# 3. Create the module
pybind11_add_module(lotm src/cpp_py_bridge.cpp)
target_include_directories(lotm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
install(TARGETS lotm DESTINATION .)
