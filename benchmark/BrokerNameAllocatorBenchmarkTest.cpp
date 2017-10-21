#include <benchmark/benchmark.h>
#include <BrokerNameAllocator.h>

static void BM_BrokerNameAllocator_valid(benchmark::State &state) {
    std::string brokerName("broker1");
    for (auto _ : state) {
        zk::BrokerNameAllocator::valid(brokerName);
    }
}

BENCHMARK(BM_BrokerNameAllocator_valid);

BENCHMARK_MAIN();