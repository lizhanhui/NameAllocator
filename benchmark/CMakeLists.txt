add_executable(BrokerNameAllocator_bt BrokerNameAllocatorBenchmarkTest.cpp)

target_link_libraries(BrokerNameAllocator_bt
        ${CMAKE_PROJECT_NAME}_base
        benchmark)

add_test(NAME BrokerNameAllocator_bt
         COMMAND BrokerNameAllocator_bt)

add_dependencies(check BrokerNameAllocator_bt)

