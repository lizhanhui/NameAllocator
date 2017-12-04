#ifndef ZOOKEEPER_BROKERNAMEALLOCATOR_H
#define ZOOKEEPER_BROKERNAMEALLOCATOR_H

#include <string>
#include <unordered_set>
#include <zookeeper/zookeeper.h>
#include <glog/logging.h>
#include <thread>

#include "ZKPaths.h"
#include "InetAddr.h"

namespace zk {
    class BrokerNameAllocator {
    public:
        BrokerNameAllocator(const std::string& broker_name_prefix, const std::string &lock_prefix, zhandle_t* handler)
                : broker_name_prefix(broker_name_prefix), lock_prefix(lock_prefix), handler(handler) {
            lock();
        }

        ~BrokerNameAllocator();

        std::string lookup(const std::string& ip);

        std::string acquire(const std::string &ip, int span, const std::string &prefer_name, int minIndex = 1000);

        bool release(const std::string &broker_name, const std::string &ip);

        static bool valid(const std::string &broker_name);

    private:
        void lock();

        void unlock();

    private:
        const std::string& broker_name_prefix;
        const std::string& lock_prefix;
        zhandle_t* handler;
    };
}

#endif //ZOOKEEPER_BROKERNAMEALLOCATOR_H
