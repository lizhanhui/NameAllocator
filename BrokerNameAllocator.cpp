#include "BrokerNameAllocator.h"

namespace zk {

    void BrokerNameAllocator::lock() {
        if (!ZKPaths::exists(handler, lock_prefix)) {
            ZKPaths::mkdirs(handler, lock_prefix);
        }

        std::string path = lock_prefix + "/lock";
        std::string ip = InetAddr::localhost();
        while (!ZKPaths::mkdir(handler, path, ip)) {
            LOG(ERROR) << "Unable to acquire lock. Sleep for 1 second";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        LOG(INFO) << "Lock acquired OK";
    }


    void BrokerNameAllocator::unlock() {
        std::string path = lock_prefix + "/lock";
        std::string ip = InetAddr::localhost();
        if (ip == ZKPaths::get(handler, path)) {
            while (!ZKPaths::rm(handler, path)) {
                LOG(ERROR) << "Failed to release lock";
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        LOG(INFO) << "Lock released OK";
    }

    std::string BrokerNameAllocator::lookup(const std::string &ip) {
        std::unordered_set<std::string> broker_name_set = ZKPaths::children(handler, broker_name_prefix);
        if (!broker_name_set.empty()) {
            for (const std::string &broker_name : broker_name_set) {
                std::string name_node = broker_name_prefix + "/" + broker_name;
                std::unordered_set<std::string> ip_set = ZKPaths::children(handler, name_node);
                for (const std::string &ip_node : ip_set) {
                    if (ip == ZKPaths::get(handler, name_node + "/" + ip_node)) {
                        return broker_name;
                    }
                }
            }
        }

        throw -1;
    }

    std::string BrokerNameAllocator::acquire(const std::string &ip, int span) {
        return "OK";
    }

    bool BrokerNameAllocator::release(const std::string &broker_name, const std::string &ip) {
        return true;
    }
}
