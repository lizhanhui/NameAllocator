cmake_minimum_required(VERSION 3.7)
project(NameAllocator)

set(CMAKE_CXX_STANDARD 11)

if (${CMAKE_SYSTEM} MATCHES "Linux")
    message("Building a static-linked executable for Linux platform")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    SET(BUILD_SHARED_LIBRARIES OFF)
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

set(SOURCE_FILES main.cpp)

SET(BROKER_NAME_ALLOCATOR_BASE_FILES
        ZKPaths.cpp
        Properties.cpp
        InetAddr.cpp
        BrokerNameAllocator.cpp)

find_package(Glog REQUIRED)
find_package(gflags REQUIRED)

include_directories(
        /usr/local/include
        ${GLOG_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include)

add_library(${CMAKE_PROJECT_NAME}_base ${BROKER_NAME_ALLOCATOR_BASE_FILES})

TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_base
        zookeeper_mt
        ${GLOG_LIBRARIES}
        gflags)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_PROJECT_NAME}_base)

add_subdirectory(tests)
