cmake_minimum_required(VERSION 2.8)

project (slib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB_RECURSE slibHeaders "slib/*.h")
add_custom_target(headers SOURCES ${slibHeaders})

set(SLIB_SOURCES slib/Numeric.cpp
                 slib/String.cpp
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
                 slib/util/CmdLine.cpp
                 slib/util/Config.cpp
                 slib/util/FileUtils.cpp
                 slib/util/JsonUtils.cpp
                 slib/util/Log.cpp
                 slib/util/PidFile.cpp
                 slib/util/ProcessUtils.cpp
                 slib/util/PropertySource.cpp
                 slib/util/Random.cpp
                 slib/util/StringUtils.cpp
                 slib/util/System.cpp
                 slib/util/SystemInfo.cpp
                 slib/StringBuilder.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -fPIC")

add_library(slib STATIC ${SLIB_SOURCES})

install (TARGETS slib DESTINATION lib)

install (DIRECTORY slib/
         DESTINATION include/slib
         FILES_MATCHING PATTERN "*.h")

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_INSTALL_PREFIX}/lib/libslib.a
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_INSTALL_PREFIX}/include/slib)
