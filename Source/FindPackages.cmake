# Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

find_package(MeddySDK_Meddyproject 0 CONFIG REQUIRED COMPONENTS Static)
find_package(MeddySDK_Meddydata 0 CONFIG REQUIRED COMPONENTS Static)
find_package(MeddySDK_DAM 0 CONFIG REQUIRED COMPONENTS Static)

find_package(CommandParser 0 CONFIG REQUIRED COMPONENTS Static)

find_package(CppUtils_StdReimpl CONFIG REQUIRED COMPONENTS Static)
find_package(CppUtils_Core CONFIG REQUIRED COMPONENTS Static)
find_package(CppUtils_Misc CONFIG REQUIRED COMPONENTS Static)
