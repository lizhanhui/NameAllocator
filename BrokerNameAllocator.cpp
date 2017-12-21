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
            zkClient.mkdirs(lock_prefix, nullptr);
        }

        std::string path = lock_prefix + "/lock";
        std::string ip = InetAddr::localhost();
        while (!zkClient.mkdir(path, ip, true)) {
            spdlog::get("logger")->error("Unable to acquire lock. Sleep for 1 second");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        spdlog::get("logger")->info("Lock acquired OK");

        if(!zkClient.exists(broker_name_prefix)) {
            zkClient.mkdirs(broker_name_prefix, nullptr);
        }
    }


    void BrokerNameAllocator::unlock() {
        std::string path = lock_prefix + "/lock";
        if (zkClient.exists(path)) {
            std::string ip = InetAddr::localhost();
            if (ip == zkClient.get(path)) {
                while (!zkClient.rm(path)) {
                    spdlog::get("logger")->error("Failed to release lock");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            spdlog::get("logger")->info("Lock released OK");
        }
    }

    std::string BrokerNameAllocator::getNodeTextValue() {
        const char *clusterName = getenv("brokerClusterName");
        if (nullptr == clusterName) {
            spdlog::get("logger")->warn("Environment variable {} is not available", "brokerClusterName");
            clusterName = "cluster1";
        }

        const char *idcRoom = getenv("current_idc");
        if (nullptr == idcRoom) {
            spdlog::get("logger")->warn("Environment variable: {} is not available", "current_idc");
            idcRoom = "Unknown";
        }

        rapidjson::StringBuffer stringBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
        writer.StartObject();
        writer.String("cluster");
        writer.String(clusterName);
        writer.String("idcRoom");
        writer.String(idcRoom);
        writer.EndObject();
        return std::string(stringBuffer.GetString());
    }

    std::string BrokerNameAllocator::lookup(const std::string &ip) {
        std::unordered_set<std::string> broker_name_set = zkClient.children(broker_name_prefix);
        if (!broker_name_set.empty()) {
            for (const std::string &broker_name : broker_name_set) {
                std::string name_node_path(broker_name_prefix);
                name_node_path.append("/").append(broker_name);
                std::unordered_set<std::string> ip_set = zkClient.children(name_node_path);
                for (const std::string &next : ip_set) {
                    if (next == ip) {
                        spdlog::get("logger")->info("Broker name found: {} for IP: {}", broker_name, ip);
                        return broker_name;
                    }
                }
            }
        } else {
            spdlog::get("logger")->info("No existing broker names so far. Quit look up broker name by IP");
        }

        throw -1;
    }

    std::string BrokerNameAllocator::acquire(const std::string &ip, unsigned int span, const std::string &prefer_name, int minIndex) {

        spdlog::get("logger")->debug("Try to acquire a new broker name. IP: {}, span: {}, preferName: {}, minIndex: {}",
                                     ip, span, prefer_name, minIndex);
        std::string broker_name;
        bool exists = true;
        try {
            broker_name = lookup(ip);
            return broker_name;
        } catch (int status) {
            if (-1 == status) {
                exists = false;
            }
        }

        bool create = true;
        int index = 0;
        if (!exists) {
            if (!prefer_name.empty()) {
                spdlog::get("logger")->info("Trying to acquire and register preferred broker name: {}", prefer_name);
                std::string prefer_name_node(broker_name_prefix);
                prefer_name_node.append("/").append(prefer_name);

                if(!zkClient.exists(prefer_name_node) ||
                   zkClient.children(prefer_name_node).size() < span) {
                    std::string ip_node(prefer_name_node);
                    ip_node.append("/").append(ip);
                    spdlog::get("logger")->debug("Prepare to create IP node: {} as this broker name does not exist or is not yet full", ip_node);
                    std::string nodeJsonText = getNodeTextValue();
                    spdlog::get("logger")->info("Node Text: {}", nodeJsonText);
                    zkClient.mkdirs(ip_node, nodeJsonText.c_str());
                    spdlog::get("logger")->debug("Create IP node: {} OK", ip_node);
                    return prefer_name;
                }
            }

            std::unordered_set<std::string> broker_name_set = zkClient.children(broker_name_prefix);
            if (!broker_name_set.empty()) {
                for (const std::string &broker_name_it : broker_name_set) {
                    if (!BrokerNameAllocator::valid(broker_name_it)) {
                        spdlog::get("logger")->warn("Skip bad broker name: {}", broker_name_it);
                        continue;
                    }

                    std::string name_node = broker_name_prefix + "/" + broker_name_it;
                    std::unordered_set<std::string> ip_set = zkClient.children(name_node);
                    if (ip_set.size() < span) {
                        create = false;
                        std::string ip_node_path(name_node);
                        ip_node_path.append("/").append(ip);
                        const std::string nodeJsonText = getNodeTextValue();
                        spdlog::get("logger")->info("IP node JSON text: {}", nodeJsonText);
                        zkClient.mkdir(ip_node_path, nodeJsonText);
                        spdlog::get("logger")->info("Create IP node OK using existing broker name: {}", broker_name_it);
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
            spdlog::get("logger")->debug("New broker name: {}, broker name prefix: {}", broker_name, broker_name_prefix);

            std::string ip_node_path(broker_name_prefix);
            ip_node_path.append("/").append(broker_name).append("/").append(ip);

            spdlog::get("logger")->debug("New IP node path: {}", ip_node_path);
            spdlog::get("logger")->debug("Prepare to create node: {}", ip_node_path);
            std::string nodeJsonText = getNodeTextValue();
            spdlog::get("logger")->info("Node text: {}", nodeJsonText);
            zkClient.mkdirs(ip_node_path, nodeJsonText.c_str());
            spdlog::get("logger")->debug("Create node: {} OK", ip_node_path);
        }

        return broker_name;
    }

    bool BrokerNameAllocator::release(const std::string &broker_name, const std::string &ip) {
        std::string ip_node_path(broker_name_prefix);
        ip_node_path.append("/").append(broker_name).append("/").append(ip);
        if (zkClient.exists(ip_node_path)) {
            spdlog::get("logger")->info("IP node to delete: {}", ip_node_path);
            return zkClient.rm(ip_node_path);
        }
        spdlog::get("logger")->error("IP node specified is not existing");
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
        spdlog::get("logger")->info("ZooKeeper client closed");
    }
}
