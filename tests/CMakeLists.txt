find_package(gtest)
include_directories(${GTEST_INCLUDE_DIRS})
add_executable(BrokerNameAllocator_ut BrokerNameAllocatorTest.cpp)
target_link_libraries(BrokerNameAllocator_ut
        ${GTEST_LIBRARIES}
        ${CMAKE_PROJECT_NAME}_base)
add_test(NAME BrokerNameAllocator_ut COMMAND BrokerNameAllocator_ut)