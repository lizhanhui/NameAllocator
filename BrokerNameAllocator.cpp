#include "BrokerNameAllocator.h"

namespace zk {

    BrokerNameAllocator::BrokerNameAllocator(const std::string &broker_name_prefix,
                                             const std::string &lock_prefix,
                                             const std::string &zk_address)
            : broker_name_prefix(broker_name_prefix),
              lock_prefix(lock_prefix),
              zkClient(zk_address) {
        lock();
    }

    void BrokerNameAllocator::lock() {
        if (!zkClient.exists(lock_prefix)) {
            zkClient.mkdirs(lock_prefix);
        }

        std::string path = lock_prefix + "/lock";
        std::string ip = InetAddr::localhost();
        while (!zkClient.mkdir(path, ip, true)) {
            LOG(ERROR) << "Unable to acquire lock. Sleep for 1 second";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        LOG(INFO) << "Lock acquired OK";

        if(!zkClient.exists(broker_name_prefix)) {
            zkClient.mkdirs(broker_name_prefix);
        }
    }


    void BrokerNameAllocator::unlock() {
        std::string path = lock_prefix + "/lock";
        if (zkClient.exists(path)) {
            std::string ip = InetAddr::localhost();
            if (ip == zkClient.get(path)) {
                while (!zkClient.rm(path)) {
                    LOG(ERROR) << "Failed to release lock";
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            LOG(INFO) << "Lock released OK";
        }
    }

    std::string BrokerNameAllocator::lookup(const std::string &ip) {
        std::unordered_set<std::string> broker_name_set = zkClient.children(broker_name_prefix);
        if (!broker_name_set.empty()) {
            for (const std::string &broker_name : broker_name_set) {
                std::string name_node_path(broker_name_prefix);
                name_node_path.append("/").append(broker_name);
                std::unordered_set<std::string> ip_set = zkClient.children(name_node_path);
                for (const std::string &ip_node : ip_set) {
                    std::string ip_node_path(name_node_path);
                    ip_node_path.append("/").append(ip_node);
                    if (ip == zkClient.get(ip_node_path)) {
                        return broker_name;
                    }
                }
            }
        }

        throw -1;
    }

    std::string BrokerNameAllocator::acquire(const std::string &ip, int span, const std::string &prefer_name, int minIndex) {
        std::string broker_name;
        bool exists = true;
        try {
            broker_name = lookup(ip);
        } catch (int status) {
            if (-1 == status) {
                exists = false;
            }
        }

        bool create = true;
        int index = 0;
        if (!exists) {

            if (!prefer_name.empty()) {
                LOG(INFO) << "Trying to acquire and register preferred broker name: " << prefer_name;
                std::string prefer_name_node(broker_name_prefix);
                prefer_name_node.append("/").append(prefer_name);

                if(!zkClient.exists(prefer_name_node) ||
                   zkClient.children(prefer_name_node).size() < span) {
                    std::string ip_node(prefer_name_node);
                    ip_node.append("/").append(ip);
                    zkClient.mkdirs(ip_node);
                    zkClient.set(ip_node, ip);
                    return prefer_name;
                }
            }

            std::unordered_set<std::string> broker_name_set = zkClient.children(broker_name_prefix);
            if (!broker_name_set.empty()) {
                for (const std::string &broker_name_it : broker_name_set) {
                    if (!BrokerNameAllocator::valid(broker_name_it)) {
                        LOG(WARNING) << "Skip bad broker name: " << broker_name_it;
                        continue;
                    }

                    std::string name_node = broker_name_prefix + "/" + broker_name_it;
                    std::unordered_set<std::string> ip_set = zkClient.children(name_node);
                    if (ip_set.size() < span) {
                        create = false;
                        std::string ip_node_path(name_node);
                        ip_node_path.append("/").append(ip);
                        zkClient.mkdir(ip_node_path, ip);
                        broker_name = broker_name_it;
                        break;
                    }
                    std::string::size_type pos = std::string("broker").size();
                    int ordinal = std::stoi(broker_name_it.substr(pos));
                    if (index < ordinal) {
                        index = ordinal;
                    }
                }
            }
        }

        if (index < minIndex) {
            index = minIndex;
        }

        if (create) {
            broker_name.clear();
            broker_name.append("broker").append(std::to_string(++index));
            std::string ip_node_path(broker_name_prefix);
            ip_node_path.append("/").append(broker_name).append("/").append(ip);
            zkClient.mkdirs(ip_node_path);
            zkClient.set(ip_node_path, ip);
        }

        return broker_name;
    }

    bool BrokerNameAllocator::release(const std::string &broker_name, const std::string &ip) {
        std::string ip_node_path(broker_name_prefix);
        ip_node_path.append("/").append(broker_name).append("/").append(ip);
        if (zkClient.exists(ip_node_path)) {
            LOG(INFO) << "IP node to delete: " << ip_node_path;
            return zkClient.rm(ip_node_path);
        }

        LOG(ERROR) << "IP node specified is not existing";
        return false;
    }

    bool BrokerNameAllocator::valid(const std::string &broker_name) {
        std::string prefix = "broker";
        if (broker_name.find(prefix) == 0) {
            try {
                std::stoi(broker_name.substr(prefix.size()));
                return true;
            } catch (...) {
                return false;
            }
        }
        return false;
    }

    BrokerNameAllocator::~BrokerNameAllocator() {
        unlock();
        LOG(INFO) << "ZooKeeper client closed";
    }
}
