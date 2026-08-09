# author: blazejchojnacki
# project: Lord_of_the_Mods

cmake_minimum_required(VERSION 3.15)
project(Lord_of_the_Mods LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lotm src/main.cpp)

target_include_directories(lotm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
