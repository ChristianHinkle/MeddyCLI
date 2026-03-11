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

project(${MY_BASE_PROJECT_NAME_FULL}_Source
  VERSION 0.1.0
  LANGUAGES CXX
  )

include(GNUInstallDirs)
include(MyGNUInstallDirs)

#
# Create targets.
#
# We namespace target names using underscore characters "_". Namespacing helps avoid ambiguity. E.g., for
# the DAM library of MeddySDK, we name it "MeddySDK_DAM" instead of just "DAM", because that would be very vague.
#

add_library(${MY_BASE_PROJECT_NAME_FULL}_Source INTERFACE)

#
# Set output names of our targets.
#
# This determines the name of the binary files generated for the target. The name may differ from the actual
# target name. E.g., for MeddyCLI, we name its executable file "meddy" to suit our desired command line usage.
#

set_target_properties(${MY_BASE_PROJECT_NAME_FULL}_Source
  PROPERTIES
    OUTPUT_NAME "meddy"
  )

#
# Add alias targets.
#
# We name these with special characters that filenames can't have. We use these aliases instead of the direct
# target names where possible so that they can't be be interpreted as filenames.
#
# It's conventional to use "::" in these names, like namespaces. If a target doesn't have anything to namespace in its
# name then just prepend "::" to it, analogous to how we indicate global scope in cpp.
#
# The aliases set up here get used by build interface users. Make sure the same aliases are also set up for install
# interface users somewhere else. We do that in the export's config file. It's important to make sure the names are
# the same in both cases for consistency. Namely, so that build interface users can use `OVERRIDE_FIND_PACKAGE` with
# `FetchContent_Declare()`.
#

add_library(${MY_BASE_PROJECT_NAME_NAMESPACE}::${MY_BASE_PROJECT_NAME_LEAFNAME}::Source ALIAS ${MY_BASE_PROJECT_NAME_FULL}_Source)

# List out required language features.
target_compile_features(${MY_BASE_PROJECT_NAME_FULL}_Source
  INTERFACE
    cxx_std_20
  )

#
# Add all header files and set up their include directories.
#
# The `BASE_DIRS` of file sets automatically get added to the target's include
# directories. So we don't use the `target_include_directories()` command anymore
# because that would be redundant.
#
# Interface and/or public file sets are important because we use them to install headers for
# the install interface users.
#
# A nice benefit of listing all header files is that they can be easily-discoverable by IDEs.
#
target_sources(${MY_BASE_PROJECT_NAME_FULL}_Source
  INTERFACE
    FILE_SET ${MY_BASE_PROJECT_NAME_FULL_LOWERCASE}_source_interface_headers
      TYPE HEADERS
      BASE_DIRS
        #
        # Base directory of private headers.
        #
        # It's nice to leave the "Source" directory out of this base include path so that, for
        # cpp include statements, you have to type "Source/" to access the non-public headers. This
        # makes it obvious for programmers when a header shouldn't be included in public files.
        #
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        # Add all private header files.
  )

# Add all source files.
block(SCOPE_FOR VARIABLES)

  # We have file lists separated into "Files.cmake" files and include them here in the generalized cmake
  # code so that code writers don't have to dig through this cmake code to add their source files.
  include("Files.cmake")

  # We expect the include to define this variable for us to use.
  if(NOT DEFINED FILES_CMAKE_RESULT)
    message(FATAL_ERROR "The files include did not define the variable `FILES_CMAKE_RESULT` for us to use.")
  endif()

  # List out all our source files, relative to our "Files" directory.
  set(MyTargetSources
    ${FILES_CMAKE_RESULT}
    )

  # Unset this variable, as it is no longer needed.
  unset(FILES_CMAKE_RESULT)

  # Add source file paths for `BUILD_LOCAL_INTERFACE` users.
  block(SCOPE_FOR VARIABLES)
    # Wrap each source file path in the conditional generator expression.
    set(MyTargetSourcesBuildLocalInterface ${MyTargetSources})
    list(TRANSFORM MyTargetSourcesBuildLocalInterface PREPEND "$<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Files/")
    list(TRANSFORM MyTargetSourcesBuildLocalInterface APPEND ">")

    target_sources(${MY_BASE_PROJECT_NAME_FULL}_Source
      INTERFACE
        ${MyTargetSourcesBuildLocalInterface}
      )
  endblock()

  # Add source file paths for `INSTALL_INTERFACE` users.
  block(SCOPE_FOR VARIABLES)
    # Wrap each source file path in the conditional generator expression.
    set(MyTargetSourcesInstallInterface ${MyTargetSources})
    list(TRANSFORM MyTargetSourcesInstallInterface PREPEND "$<INSTALL_INTERFACE:${MY_GNU_INSTALL_SRCDIR}/${PROJECT_NAME}/")
    list(TRANSFORM MyTargetSourcesInstallInterface APPEND ">")

    target_sources(${MY_BASE_PROJECT_NAME_FULL}_Source
      INTERFACE
        ${MyTargetSourcesInstallInterface}
      )
  endblock()

endblock()

# Get any package exports which we depend on. This gives us imported targets for them.

find_package(MeddySDK_Meddyproject 0 CONFIG REQUIRED COMPONENTS Static)

find_package(Boost 0 CONFIG REQUIRED COMPONENTS filesystem)

find_package(CppUtils_StdReimpl CONFIG REQUIRED COMPONENTS Static)
find_package(CppUtils_Core CONFIG REQUIRED COMPONENTS Static)
find_package(CppUtils_Misc CONFIG REQUIRED COMPONENTS Static)

# Add dependencies to other targets. For imported targets, it's important to only depend on them if we
# are BUILD_LOCAL_INTERFACE, because we don't want them to get mixed into our export.
target_link_libraries(${MY_BASE_PROJECT_NAME_FULL}_Source
  INTERFACE
    $<BUILD_LOCAL_INTERFACE:MeddySDK::Meddyproject::Static>
    $<BUILD_LOCAL_INTERFACE:Boost::filesystem>
    $<BUILD_LOCAL_INTERFACE:CppUtils::StdReimpl::Static>
    $<BUILD_LOCAL_INTERFACE:CppUtils::Core::Static>
    $<BUILD_LOCAL_INTERFACE:CppUtils::Misc::Static>
  )

# Associate our targets with an export and specify how their files would be installed.
install(TARGETS ${MY_BASE_PROJECT_NAME_FULL}_Source
  EXPORT "${MY_BASE_PROJECT_NAME_FULL}Export"
  COMPONENT "Source"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}"
  #
  # Add to interface include directories.
  #
  # Note: We take care of install-specific include directories here instead of `target_include_directories()`.
  #
  # Note: It's important to make sure cpp include statements of public headers are compatible between
  # install interface users and build interface users. E.g., a user's include statements should work for
  # both packaged library usage and subproject library usage, without having to adapt their code to either use case.
  #
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
  # Copy over any interface-facing headers.
  FILE_SET ${MY_BASE_PROJECT_NAME_FULL_LOWERCASE}_source_interface_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
  )

# Specify how installing should copy our source files to the install directory. Note: The destination must
# match with the `target_sources` paths that we used.
install(
  FILES $<TARGET_PROPERTY:${MY_BASE_PROJECT_NAME_FULL}_Source,INTERFACE_SOURCES>
  DESTINATION "${MY_GNU_INSTALL_SRCDIR}/${PROJECT_NAME}"
  )
