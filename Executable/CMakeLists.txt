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

project(${MY_BASE_PROJECT_NAME_FULL}_Executable
  VERSION 0.1.0
  LANGUAGES CXX
  )

include(GNUInstallDirs)

#
# Create targets.
#
# We namespace target names using underscore characters "_". Namespacing helps avoid ambiguity. E.g., for
# the DAM library of MeddySDK, we name it "MeddySDK_DAM" instead of just "DAM", because that would be very vague.
#

add_executable(${MY_BASE_PROJECT_NAME_FULL}_Executable)

#
# Set output names of our targets.
#
# This determines the name of the binary files generated for the target. The name may differ from the actual
# target name. E.g., for MeddyCLI, we name its executable file "meddy" to suit our desired command line usage.
#

set_target_properties(${MY_BASE_PROJECT_NAME_FULL}_Executable
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

add_executable(${MY_BASE_PROJECT_NAME_NAMESPACE}::${MY_BASE_PROJECT_NAME_LEAFNAME}::Executable ALIAS ${MY_BASE_PROJECT_NAME_FULL}_Executable)

# List out required language features.
target_compile_features(${MY_BASE_PROJECT_NAME_FULL}_Executable
  PUBLIC
    cxx_std_20
  )

# Note that we do not "find package" for our parent project. We don't need to since we are built in the same
# cmake invocation as the it. That means we're being processed during the same configuration step as them, which
# means we'll have all their targets. Also, the targets that we reference in `target_link_libraries` commands don't
# even need to exist yet because CMake only validates the existence of targets at the end of the entire
# configuration process.

# Add dependencies to other targets. For imported targets, it's important to only depend on them if we
# are BUILD_LOCAL_INTERFACE, because we don't want them to get mixed into our export.
target_link_libraries(${MY_BASE_PROJECT_NAME_FULL}_Executable
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${MY_BASE_PROJECT_NAME_NAMESPACE}::${MY_BASE_PROJECT_NAME_LEAFNAME}::Source>
  )

# Associate our targets with an export and specify how their files would be installed.
install(TARGETS ${MY_BASE_PROJECT_NAME_FULL}_Executable
  EXPORT "${MY_BASE_PROJECT_NAME_FULL}Export"
  COMPONENT "Executable"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
  # Install our runtime executable in the base "bin" dir, since that's where our PATH environment variable looks
  # at when installed from NSIS. If we put our executable in a subdir of this, it won't be found when executing
  # the command.
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  )
