cmake_minimum_required(VERSION 2.8)

project (slib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

SET(CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB_RECURSE slibHeaders "slib/*.h" "slib/*.hpp")
add_custom_target(headers SOURCES ${slibHeaders})

option(WITH_JSON "build JSON utils" ON)
option(WITH_TESTS "compile tests" OFF)
option(WITH_COVERAGE "enable code coverage" OFF)

set(SLIB_SOURCES
	slib/lang/Class.cpp
	slib/lang/Numeric.cpp
	slib/lang/Object.cpp
	slib/lang/String.cpp
	slib/lang/StringBuilder.cpp
	slib/lang/StringView.cpp
	slib/collections/Properties.cpp
	slib/concurrent/Semaphore.cpp
	slib/concurrent/Thread.cpp
	slib/concurrent/FdThread.cpp
	slib/exception/Exception.cpp
	slib/io/IO.cpp
	slib/io/InputStream.cpp
	slib/io/FileInputStream.cpp
	slib/io/OutputStream.cpp
	slib/io/FileOutputStream.cpp
	slib/text/StringCharacterIterator.cpp
	slib/util/CmdLine.cpp
	slib/util/Config.cpp
	slib/util/FileUtils.cpp
	slib/util/Log.cpp
	slib/util/PidFile.cpp
	slib/util/ProcessUtils.cpp
	slib/util/PropertySource.cpp
	slib/util/Random.cpp
	slib/util/StringUtils.cpp
	slib/util/System.cpp
	slib/util/SystemInfo.cpp
	slib/util/TemplateUtils.cpp
	slib/util/expr/Builtins.cpp
	slib/util/expr/Expression.cpp
	slib/util/expr/ExpressionEvaluator.cpp
	slib/util/expr/ExpressionFormatter.cpp
	slib/util/expr/ExpressionInputStream.cpp
	slib/util/expr/Function.cpp
	slib/util/expr/Resolver.cpp
	)

if(WITH_JSON)
	MESSAGE("JSON utilities enabled")
	list(APPEND SLIB_SOURCES slib/util/JsonUtils.cpp)
endif(WITH_JSON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -fPIC")

if(WITH_TESTS)
	MESSAGE("Tests enabled")
	add_subdirectory(tests)
endif(WITH_TESTS)

if(WITH_COVERAGE)
	include(CodeCoverage)
	append_coverage_compiler_flags()
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
	setup_target_for_coverage_lcov(
		NAME coverage
		EXECUTABLE slib_tests
		EXCLUDE "/usr/include/*"
	)
endif(WITH_COVERAGE)

add_library(slib STATIC ${SLIB_SOURCES})

install (TARGETS slib DESTINATION lib)

install (DIRECTORY slib/
	     DESTINATION include/slib
		 FILES_MATCHING PATTERN "*.h")

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_INSTALL_PREFIX}/lib/libslib.a
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_INSTALL_PREFIX}/include/slib)
