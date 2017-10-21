#include "BrokerNameAllocator.h"
#include <gtest/gtest.h>

TEST(test_valid, normal) {
    std::string brokerName("broker0");
    EXPECT_TRUE(zk::BrokerNameAllocator::valid(brokerName));
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}