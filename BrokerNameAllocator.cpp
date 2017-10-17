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
                std::string name_node_path(broker_name_prefix);
                name_node_path.append("/").append(broker_name);
                std::unordered_set<std::string> ip_set = ZKPaths::children(handler, name_node_path);
                for (const std::string &ip_node : ip_set) {
                    std::string ip_node_path(name_node_path);
                    ip_node_path.append("/").append(ip_node);
                    if (ip == ZKPaths::get(handler, ip_node_path)) {
                        return broker_name;
                    }
                }
            }
        }

        throw -1;
    }

    std::string BrokerNameAllocator::acquire(const std::string &ip, int span, const std::string &prefer_name) {
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

                if(!ZKPaths::exists(handler, prefer_name_node) ||
                   ZKPaths::children(handler, prefer_name_node).size() < span) {
                    std::string ip_node(prefer_name_node);
                    ip_node.append("/").append(ip);
                    ZKPaths::mkdirs(handler, ip_node);
                    ZKPaths::set(handler, ip_node, ip);
                    return prefer_name;
                }
            }

            std::unordered_set<std::string> broker_name_set = ZKPaths::children(handler, broker_name_prefix);
            if (!broker_name_set.empty()) {
                for (const std::string &broker_name_it : broker_name_set) {
                    if (!valid(broker_name_it)) {
                        LOG(WARNING) << "Skip bad broker name: " << broker_name_it;
                        continue;
                    }

                    std::string name_node = broker_name_prefix + "/" + broker_name_it;
                    std::unordered_set<std::string> ip_set = ZKPaths::children(handler, name_node);
                    if (ip_set.size() < span) {
                        create = false;
                        std::string ip_node_path(name_node);
                        ip_node_path.append("/").append(ip);
                        ZKPaths::mkdir(handler, ip_node_path, ip);
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

        if (create) {
            broker_name.clear();
            broker_name.append("broker").append(std::to_string(++index));
            std::string ip_node_path(broker_name_prefix);
            ip_node_path.append("/").append(broker_name).append("/").append(ip);
            ZKPaths::mkdirs(handler, ip_node_path);
            ZKPaths::set(handler, ip_node_path, ip);
        }

        return broker_name;
    }

    bool BrokerNameAllocator::release(const std::string &broker_name, const std::string &ip) {
        std::string ip_node_path(broker_name_prefix);
        ip_node_path.append("/").append(broker_name).append("/").append(ip);
        if (ZKPaths::exists(handler, ip_node_path)) {
            LOG(INFO) << "IP node to delete: " << ip_node_path;
            return ZKPaths::rm(handler, ip_node_path);
        }

        LOG(ERROR) << "IP node specified is not existing";
        return false;
    }

    bool BrokerNameAllocator::valid(const std::string &broker_name) {
        return std::regex_match(broker_name, broker_name_pattern);
    }
}
