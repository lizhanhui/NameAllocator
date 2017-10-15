#include "BrokerNameAllocator.h"

namespace zk {

    std::string BrokerNameAllocator::acquire(const std::string &ip, int span) {


        return "OK";
    }

    bool BrokerNameAllocator::release(const std::string &broker_name, const std::string &ip) {
        return true;
    }
}
