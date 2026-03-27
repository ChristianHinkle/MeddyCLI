# Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

cmake_minimum_required(VERSION 4.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeProjectFramework/CMakeProjectFramework.cmake)

cmakeprojectframework_cmakelists_mainproject_executable("MeddyCLI" "" "MeddyCLI" "https://github.com/ChristianHinkle/MeddyCLI")
