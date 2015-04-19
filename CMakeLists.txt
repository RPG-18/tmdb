set(PROJECT tmdb)
project(${PROJECT} C CXX)
cmake_minimum_required(VERSION 2.8.11)

option(TEST_ENABLE "Enable tests" OFF)

add_definitions(-std=c++11)

add_subdirectory(3rdparty/leveldb-1.18)

include_directories(3rdparty/leveldb-1.18/include
                    src
)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)

find_package (Boost 1.55.0 COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(src)

if(TEST_ENABLE)
    enable_testing()
    add_subdirectory(3rdparty/gtest-1.7.0)
    include_directories(3rdparty/gtest-1.7.0/include
    )
    add_subdirectory(test)
endif()