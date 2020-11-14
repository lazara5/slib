cmake_policy(SET CMP0015 NEW)

if(DEFINED ENV{CPPUTEST_HOME})
	message(STATUS "Using CppUTest home: $ENV{CPPUTEST_HOME}")
	set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
	set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
	set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
	find_package(PkgConfig REQUIRED)
	pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
	message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
endif()

add_compile_options(-DCPPUTEST_MEM_LEAK_DETECTION_DISABLED)

set(TESTS_SOURCES
	Global.h
	AllTests.cpp
	TestConfig.cpp
	TestExpression.cpp
	TestTypeSystem.cpp
	TestNumeric.cpp
)

configure_file(data/test.conf ${CMAKE_CURRENT_BINARY_DIR}/data/test.conf COPYONLY)

include_directories(${CPPUTEST_INCLUDE_DIRS})
link_directories(${CPPUTEST_LIBRARIES})

add_executable(slib_tests ${TESTS_SOURCES})
target_link_libraries(slib_tests slib fmt pthread ${CPPUTEST_LDFLAGS} gcov)
