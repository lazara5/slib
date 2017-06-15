cmake_minimum_required(VERSION 2.8)

project (slib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SLIB_SOURCES slib/concurrent/Semaphore.cpp
                 slib/concurrent/Thread.cpp
                 slib/exception/Exception.cpp
                 slib/util/CmdLine.cpp
                 slib/util/Config.cpp
                 slib/util/FileUtils.cpp
                 slib/util/Log.cpp
                 slib/util/PidFile.cpp
                 slib/util/ProcessUtils.cpp
                 slib/util/Random.cpp
                 slib/util/StringUtils.cpp
                 slib/util/System.cpp
                 slib/StringBuilder.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -fPIC")

add_library(slib STATIC ${SLIB_SOURCES})


