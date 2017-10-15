cmake_minimum_required(VERSION 3.8)
project(NameAllocator)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

set(SOURCE_FILES main.cpp
        ZKPaths.cpp
        Properties.cpp
        InetAddr.cpp
        BrokerNameAllocator.cpp)

find_package(Glog REQUIRED)

include_directories(
        /usr/local/include
        ${GLOG_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME}
        zookeeper_mt
        ${GLOG_LIBRARIES})
