# Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

# Write out the main project name here, in one place, to make renaming easier. This is mainly used by our
# boilerplate code. There may be other mentions of the project's name mixed in certain places of this file, so
# don't rely on these variables alone for renaming.
set(my_project_name "MeddyCLI")
set(my_project_name_target_alias_format "::MeddyCLI")

include(CMakeFindDependencyMacro)

find_dependency(MeddySDK_Meddyproject CONFIG COMPONENTS Static)
find_dependency(MeddySDK_Meddydata CONFIG COMPONENTS Static)

find_dependency(Boost CONFIG COMPONENTS filesystem)

find_dependency(CppUtils_StdReimpl CONFIG COMPONENTS Static)
find_dependency(CppUtils_Core CONFIG COMPONENTS Static)
find_dependency(CppUtils_Misc CONFIG COMPONENTS Static)

# Include our export. This imports all of our targets.
include("${CMAKE_CURRENT_LIST_DIR}/${my_project_name}Export.cmake")

#
# Add alias targets whose names match those from the project file.
#
# It's important to make sure the names are the same for consistency. Namely, so that build
# interface users can use `OVERRIDE_FIND_PACKAGE` with `FetchContent_Declare()`.
#

add_executable(${my_project_name_target_alias_format}::Include ALIAS ${my_project_name}_Include)
add_executable(${my_project_name_target_alias_format}::Source ALIAS ${my_project_name}_Source)
add_executable(${my_project_name_target_alias_format}::Executable ALIAS ${my_project_name}_Executable)
