#ifndef ZOOKEEPER_BROKERNAMEALLOCATOR_H
#define ZOOKEEPER_BROKERNAMEALLOCATOR_H

#include <string>
#include <unordered_set>
#include <zookeeper/zookeeper.h>
#include <thread>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "spdlog/spdlog.h"

#include "ZKClient.h"
#include "InetAddr.h"

namespace zk {
    class BrokerNameAllocator {

    public:
        BrokerNameAllocator(const std::string& broker_name_prefix,
                            const std::string &lock_prefix,
                            const std::string &zk_address);

        ~BrokerNameAllocator();

        std::string lookup(const std::string& ip);

        std::string acquire(const std::string &ip, int span, const std::string &prefer_name, int minIndex = 1000);

        bool release(const std::string &broker_name, const std::string &ip);

        static bool valid(const std::string &broker_name);

    private:

        void lock();

        void unlock();

        std::string getNodeTextValue();

        const std::string& broker_name_prefix;
        const std::string& lock_prefix;
        ZKClient zkClient;
    };
}

#endif //ZOOKEEPER_BROKERNAMEALLOCATOR_H
