#ifndef ZOOKEEPER_BROKERNAMEALLOCATOR_H
#define ZOOKEEPER_BROKERNAMEALLOCATOR_H

#include <string>

namespace zk {
    class BrokerNameAllocator {
    public:
        BrokerNameAllocator(const std::string& prefix) : prefix(prefix) {
        }

        std::string acquire(const std::string &ip, int span);

        bool release(const std::string& broker_name, const std::string& ip);

    private:
        const std::string& prefix;
    };
}

#endif //ZOOKEEPER_BROKERNAMEALLOCATOR_H
