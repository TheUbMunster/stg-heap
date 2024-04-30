cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0135 NEW)
project(global-project)
add_subdirectory(stg-heap)
add_subdirectory(unit-tests)

#googletest requirements: https://google.github.io/googletest/quickstart-cmake.html
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#three cases:
#1. build the heap lib (win or linux)
#2. build the unit tests for debugging
#3. build the unit tests for coverage
set(BUILD_MODE DEBUG CACHE STRING "What type of build")
set_property(CACHE BUILD_MODE PROPERTY STRINGS RELEASE_WIN RELEASE_LINUX DEBUG COVERAGE)
