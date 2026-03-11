# Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

cmake_minimum_required(VERSION 4.0)

# Write out the main project name here, in one place, to make renaming easier. This is mainly used by our
# boilerplate code. There may be other mentions of the project's name mixed in certain places of this file, so
# don't rely on these variables alone for renaming.
SET(MY_BASE_PROJECT_NAME_FULL "MeddyCLI")
string(TOLOWER ${MY_BASE_PROJECT_NAME_FULL} MY_BASE_PROJECT_NAME_FULL_LOWERCASE)
string(TOUPPER ${MY_BASE_PROJECT_NAME_FULL} MY_BASE_PROJECT_NAME_FULL_UPPERCASE)
SET(MY_BASE_PROJECT_NAME_NAMESPACE "")
SET(MY_BASE_PROJECT_NAME_LEAFNAME "MeddyCLI")

project(${MY_BASE_PROJECT_NAME_FULL}
  VERSION 0.1.0
  DESCRIPTION ""
  HOMEPAGE_URL "https://github.com/ChristianHinko/MeddyCLI"
  LANGUAGES CXX
  )

# Tell CMake where our cmake scripts ("modules") are.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GeneralCMakeModules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GeneralCMakeModules/CPack")

# Set up testing. Defines `BUILD_TESTING` which is ON by default. This must be called by the top-level project only.
if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
endif()

include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Declare subprojects.

FetchContent_Declare(${MY_BASE_PROJECT_NAME_FULL}_Source
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source"
  )

FetchContent_Declare(${MY_BASE_PROJECT_NAME_FULL}_Executable
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Executable"
  )

if(BUILD_TESTING)
  FetchContent_Declare(${MY_BASE_PROJECT_NAME_FULL}_Tests
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
    )
endif()

# Add subprojects.

FetchContent_MakeAvailable(${MY_BASE_PROJECT_NAME_FULL}_Source)

FetchContent_MakeAvailable(${MY_BASE_PROJECT_NAME_FULL}_Executable)

if(BUILD_TESTING)
  FetchContent_MakeAvailable(${MY_BASE_PROJECT_NAME_FULL}_Tests)
endif()

# Generate script that imports targets associated with our export.
install(EXPORT "${PROJECT_NAME}Export"
  FILE "${PROJECT_NAME}Export.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )

# Create a package version file for our export.
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
  )

# Copy any files for downstream usage to the install directory.
install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )

# Set some simple values for CPack based on this current project. The default
# values for these are specific to the top-level project but we want to allow for
# having several projects generate their own CPack configs without conflicting.
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")

# Tell CMake to output the generated CPack configs in a directory specific to this project. This is
# nice so we can generate our own config without conflicting with other projects.
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/CPackConfig.cmake")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/CPackSourceConfig.cmake")

#
# Use NSIS' feature for appending our binary directory to the PATH environment
# variable.
#
# Important: Make sure you modify your NSIS installation with the files from the "strlen_8192" version before
# running CPack! Or else the installer will fail to modify the PATH env var for most users, because
# variables in NSIS cannot exceed 1024 characters by default, and PATH env vars tend to be very lengthy.
#
# See our custom NSIS template where we make sure the "PATH" environment variable is
# modified by default: "CMake/Modules/CPack/NSIS.InstallOptions.ini.in".
#
# TODO: I would like to have this functionality for other generators as well, but I
# can't seem to find a generator-agnostic solution to this. E.g., in CMake's source
# code, they re-implement this feature specifically for their WIX generator. I believe this
# kind of per-generator implementation would be the "correct" way. Alternatively, it might
# be possible to invoke a script that sets the env var manually after the generator finishes
# installation of the package, but I would want to be transparent about that and give the
# user control over that behavior; not to mention, it would also have to do the opposite
# on uninstall.
#
set(CPACK_NSIS_MODIFY_PATH "ON")

# Generate a CPackConfig.cmake file for this project.
include(CPack)
