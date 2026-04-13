# Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

# Write out the main project name here, in one place, to make renaming easier. This is mainly used by our
# boilerplate code. There may be other mentions of the project's name mixed in certain places of this file, so
# don't rely on these variables alone for renaming.
SET(MY_BASE_PROJECT_NAME_FULL "MeddyCLI")
string(TOLOWER ${MY_BASE_PROJECT_NAME_FULL} MY_BASE_PROJECT_NAME_FULL_LOWERCASE)
string(TOUPPER ${MY_BASE_PROJECT_NAME_FULL} MY_BASE_PROJECT_NAME_FULL_UPPERCASE)
SET(MY_BASE_PROJECT_NAME_NAMESPACE "")
SET(MY_BASE_PROJECT_NAME_LEAFNAME "MeddyCLI")

include(CMakeFindDependencyMacro)

find_dependency(MeddySDK_Meddyproject CONFIG COMPONENTS Static)
find_dependency(MeddySDK_Meddydata CONFIG COMPONENTS Static)

find_dependency(Boost CONFIG COMPONENTS filesystem)

find_dependency(CppUtils_StdReimpl CONFIG COMPONENTS Static)
find_dependency(CppUtils_Core CONFIG COMPONENTS Static)
find_dependency(CppUtils_Misc CONFIG COMPONENTS Static)

# Include our export. This imports all of our targets.
include("${CMAKE_CURRENT_LIST_DIR}/${MY_BASE_PROJECT_NAME_FULL}Export.cmake")

#
# Add alias targets whose names match those from the project file.
#
# It's important to make sure the names are the same for consistency. Namely, so that build
# interface users can use `OVERRIDE_FIND_PACKAGE` with `FetchContent_Declare()`.
#

add_executable(${MY_BASE_PROJECT_NAME_NAMESPACE}::${MY_BASE_PROJECT_NAME_LEAFNAME}::Source ALIAS ${MY_BASE_PROJECT_NAME_FULL}_Source)
add_executable(${MY_BASE_PROJECT_NAME_NAMESPACE}::${MY_BASE_PROJECT_NAME_LEAFNAME}::Executable ALIAS ${MY_BASE_PROJECT_NAME_FULL}_Executable)
