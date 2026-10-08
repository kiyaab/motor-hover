cmake_minimum_required(VERSION 3.20)
project(HoverBikePhysicsTests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Source/HoverRacer
)

add_executable(HoverBikePhysicsHarness HoverBikePhysicsHarness.cpp)
