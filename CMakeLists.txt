cmake_minimum_required(VERSION 3.00)

set(ARCH "amd64" CACHE STRING "Architecture to build (amd64 ia32)")

include(cmake/toolchain.cmake)

project(libefi C)
add_subdirectory(libs)
add_subdirectory(progs)
