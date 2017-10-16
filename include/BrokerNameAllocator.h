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

        ~BrokerNameAllocator() {
            unlock();
            zookeeper_close(handler);
            LOG(INFO) << "ZooKeeper client closed";
        }

        std::string lookup(const std::string& ip);

        std::string acquire(const std::string &ip, int span);

        bool release(const std::string& broker_name, const std::string& ip);

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
