#include <iostream>
#include <glog/logging.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include "Properties.h"
#include "BrokerNameAllocator.h"

using namespace std;

std::vector<std::string> split(const std::string &s, char c) {
    std::vector<std::string> v;
    std::string item;
    for(int i = 0; i < s.length(); i++) {
        if (s[i] == c) {
            v.push_back(item);
            item.clear();
        } else {
            item.append(1, s[i]);
        }
    }

    if(!item.empty()) {
        v.push_back(item);
    }
    return v;
}

std::string build_zk_address() {

    const char* pZkHost = getenv("middleware_zk_hosts");
    const char* pZkPort = getenv("middleware_zk_port");
    if (nullptr == pZkHost || nullptr == pZkPort) {
        LOG(WARNING) << "Environment variable: middleware_zk_hosts or middleware_zk_port is undefined."
                     << " Using localhost:2181 by default";
        return "localhost:2181";
    }

    std::string zk_host_ip_csv(pZkHost);
    std::string zk_port(pZkPort);

    std::string zk_address;
    std::vector<std::string> segments = split(zk_host_ip_csv, ',');
    for (const std::string &item : segments) {
        if (!zk_address.empty()) {
            zk_address.append(1, ',');
        }
        zk_address.append(item).append(1, ':').append(zk_port);
    }

    LOG(INFO) << "ZooKeeper addresses:" << zk_address;
    return zk_address;

}

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    const char* user_home = getenv("HOME");
    const char* log_file_name_prefix = string(user_home).append("/name_allocator_zk_").c_str();
    google::SetLogDestination(google::GLOG_INFO, log_file_name_prefix);

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    writer.String("idc");
    writer.String("unknown");
    writer.String("ip");
    writer.String(zk::InetAddr::localhost().c_str());
    writer.EndObject();

    const char* json = sb.GetString();
    cout << json << endl;

    rapidjson::Document d;
    d.Parse(json);

    rapidjson::Value& ipV = d["ip"];
    cout << "IP:" << ipV.GetString() << endl;

    const std::string zk_address = build_zk_address();

    int span = 2;
    if (argc >= 2) {
        span = std::stoi(std::string(argv[1]));
    }

    zk::BrokerNameAllocator brokerNameAllocator("/mq/brokerNames", "/mq", zk_address);

    const string broker_conf_path = std::string(user_home) + "/rmq/conf/broker.conf";
    zk::Properties properties;
    properties.load(broker_conf_path);

    std::string ip = zk::InetAddr::localhost();

    if (argc >= 3) {
        ip = std::string(argv[2]);
    }

    std::string broker_name;

    bool okay = false;
    try {
        broker_name = brokerNameAllocator.lookup(ip);
        okay = true;
    } catch (int status) {
        LOG(INFO) << "Cannot find broker name record according to IP: " << ip;
    }

    if (!okay) {
        string key = "brokerName";
        if (properties.has(key)) {
            broker_name = properties.get(key);
            if (zk::BrokerNameAllocator::valid(broker_name)) {
                std::string allocated_broker_name = brokerNameAllocator.acquire(ip, span, broker_name);
                if (allocated_broker_name == broker_name) {
                    LOG(INFO) << "Preferred broker name is accepted and registered to ZooKeeper";
                } else {
                    broker_name = allocated_broker_name;
                }
                okay = true;
            }
        }
    }

    if (!okay) {
        broker_name = brokerNameAllocator.acquire(ip, span, "");
        okay = true;
    }

    if (okay) {
        // output the registered broker name in case this application is used in command line.
        std::cout << broker_name;
    }

    google::ShutdownGoogleLogging();

    return 0;
}