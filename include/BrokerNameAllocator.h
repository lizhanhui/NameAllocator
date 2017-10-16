#ifndef ZOOKEEPER_BROKERNAMEALLOCATOR_H
#define ZOOKEEPER_BROKERNAMEALLOCATOR_H

#include <string>
#include <zookeeper/zookeeper.h>

namespace zk {
    class BrokerNameAllocator {
    public:
        BrokerNameAllocator(const std::string& prefix, zhandle_t* handler) : prefix(prefix), handler(handler) {
        }

        std::string lookup(const std::string& ip);

        std::string acquire(const std::string &ip, int span);

        bool release(const std::string& broker_name, const std::string& ip);

    private:
        const std::string& prefix;
        zhandle_t* handler;
    };
}

#endif //ZOOKEEPER_BROKERNAMEALLOCATOR_H
