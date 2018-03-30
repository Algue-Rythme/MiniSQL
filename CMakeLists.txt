cmake_minimum_required(VERSION 3.9.3)

project(MiniSQL)

set(EXECUTABLE_OUTPUT_PATH bin/)

file (GLOB_RECURSE source_files
        src/*.cpp
	    src/*.hpp
)

add_executable(minisql ${source_files})

find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
target_link_libraries(minisql ${Boost_LIBRARIES})

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++14 -Wall -pedantic-errors)
    add_definitions(-O3 -s -DNDEBUG)
    add_definitions(-DBOOST_SPIRIT_USE_PHOENIX_V3)
    add_definitions(${GCC_COVERAGE_LINK_FLAGS}$)
endif()

set(CMAKE_BUILD_TYPE Release)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
