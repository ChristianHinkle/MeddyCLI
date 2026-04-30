# Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

cmake_minimum_required(VERSION 4.0)

message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message("MY_MEDDY_EXECUTABLE_PATH: ${MY_MEDDY_EXECUTABLE_PATH}")
message("MY_TEST_WORKING_DIRECTORY: ${MY_TEST_WORKING_DIRECTORY}")

include(ConditionallyWarnAboutDoubleQuotesInPathVariable)

conditionally_warn_about_double_quotes_in_path_variable(MY_MEDDY_EXECUTABLE_PATH)
conditionally_warn_about_double_quotes_in_path_variable(MY_TEST_WORKING_DIRECTORY)

# Delete our test files from previous test runs.
file(REMOVE_RECURSE "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase")

# Create a directory for us to test creating a new project on.
file(MAKE_DIRECTORY "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir")

# Create a bunch of dummy files as an example of important user data that should be untouched.
file(TOUCH "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/HereIsAnExampleTextFileNextToTheProject.txt")
file(TOUCH "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/MyExampleTextFile.txt")
file(MAKE_DIRECTORY "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/MyOtherCoolTextFiles")
file(TOUCH "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/MyOtherCoolTextFiles/Yo.txt")
file(TOUCH "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/MyOtherCoolTextFiles/Hey.txt")

# Invoke the c++ test program.
execute_process(
  COMMAND "${MY_MEDDY_EXECUTABLE_PATH}"
    "project"
    "new"
    "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir"
  WORKING_DIRECTORY "${MY_TEST_WORKING_DIRECTORY}"
  RESULT_VARIABLE ResultVariable
  COMMAND_ECHO STDOUT
  )

message("ResultVariable: ${ResultVariable}")

# Exit now if the c++ program failed.
if(NOT ${ResultVariable} STREQUAL 0)
  message("The c++ program exited with an error status.")
  cmake_language(EXIT ${ResultVariable})
endif()

# Perform the rest of the test, verifying that the project files exist.

if(NOT EXISTS "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/_meddyproject")
  message("Failed. Meddyproject dir does not exist.")
  cmake_language(EXIT 4)
endif()

if(NOT EXISTS "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/_meddyproject/manifest.json")
  message("Failed. Project manifest file does not exist.")
  cmake_language(EXIT 5)
endif()

if(NOT EXISTS "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/HereIsAnExampleTextFileNextToTheProject.txt")
  message("Failed. A user's file \"HereIsAnExampleTextFileNextToTheProject.txt\" has somehow been deleted during the c++ function!")
  cmake_language(EXIT 6)
endif()

if(NOT EXISTS "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/MyExampleTextFile.txt")
  message("Failed. A user's file \"MyProjectDir/MyExampleTextFile.txt\" has somehow been deleted during the c++ function!")
  cmake_language(EXIT 7)
endif()

if(NOT EXISTS "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/MyOtherCoolTextFiles")
  message("Failed. A user's file directory \"MyProjectDir/MyOtherCoolTextFiles\" has somehow been deleted during the c++ function!")
  cmake_language(EXIT 8)
endif()

if(NOT EXISTS "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/MyOtherCoolTextFiles/Yo.txt")
  message("Failed. A user's file \"MyProjectDir/MyOtherCoolTextFiles/Yo.txt\" has somehow been deleted during the c++ function!")
  cmake_language(EXIT 9)
endif()

if(NOT EXISTS "${MY_TEST_WORKING_DIRECTORY}/MyTestGeneratedFiles/Project/New/SuccessCase/MyProjectDir/MyOtherCoolTextFiles/Hey.txt")
  message("Failed. A user's file \"MyProjectDir/MyOtherCoolTextFiles/Hey.txt\" has somehow been deleted during the c++ function!")
  cmake_language(EXIT 10)
endif()

# Success.
cmake_language(EXIT 0)
