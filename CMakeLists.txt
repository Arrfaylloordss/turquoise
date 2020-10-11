# Copyright Andrey Lifanov 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0048 NEW) # new policy for project version

project(turquoise VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message (STATUS "C++ compiler is ${CMAKE_CXX_COMPILER}")
message (STATUS "CMake prefix path: ${CMAKE_PREFIX_PATH}")
set (CMAKE_LIBRARY_PATH ${CMAKE_PREFIX_PATH}/lib CACHE STRING "")
set (CMAKE_INCLUDE_PATH ${CMAKE_PREFIX_PATH}/include CACHE STRING "")
message (STATUS "CMake lib paths: ${CMAKE_LIBRARY_PATH}")
message (STATUS "CMake include paths: ${CMAKE_INCLUDE_PATH}")

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)

    add_compile_options(-s -Wall -Wextra
            -Wsign-conversion -Wduplicated-branches -Wduplicated-cond
            -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual
            -Wsign-promo -Wfloat-equal -Wshadow=compatible-local
            -Wcast-qual -Wextra-semi -Wlogical-op -Wredundant-decls
            )

elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)

    # TODO: add flags

elseif(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)

    # TODO: add flags

endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

# Targets

add_library(turquoise src/turquoise.hpp src/turquoise.cpp src/turquoise-details.hpp)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
