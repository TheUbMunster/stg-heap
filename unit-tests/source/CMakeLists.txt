set(CAL_FLAGS "-g -O0 --coverage")
#includes for google test
include_directories(${global-project_SOURCE_DIR}/googletest/googletest/include)
#includes for the unit testing project
include_directories(${global-project_SOURCE_DIR}/unit-tests/include)
#includes for the heap lib project
include_directories(${global-project_SOURCE_DIR}/stg-heap/include)
link_directories(${global-project_SOURCE_DIR}/stg-heap/source)
add_executable(unit-tests Testing.cpp)
target_link_libraries(unit-tests stg-heap GTest::gtest_main)
if (MSVC)
	message(WARNING "Using MSVC compiler, code coverage not supported")
else()
	target_compile_options(unit-tests ${CAL_FLAGS})
endif()