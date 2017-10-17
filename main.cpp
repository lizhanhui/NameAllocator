#include <iostream>
#include <glog/logging.h>
#include <cstdlib>

#include "Properties.h"
#include "BrokerNameAllocator.h"
#include "ZKPaths.h"
#include "InetAddr.h"

using namespace std;

void watcher(zhandle_t* handler, int type, int state, const char* path, void* watcherCtx) {

}

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    google::SetStderrLogging(google::GLOG_INFO);

    const char* user_home = getenv("HOME");
    google::SetLogDestination(google::GLOG_INFO, string(user_home).append("/logs/zk_").c_str());

    const string s = std::string(user_home) + "/rmq/conf/broker.conf";
    zk::Properties properties;
    properties.load(s);

    string key = "brokerName";

    if (properties.has(key)) {
        std::cout << "brokerName=" << properties.get(key) << std::endl;
    }

    const char* env_key_zk_addr = "zk_addr";
    char buffer[512];
    zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
    static zhandle_t *zh;
    const char* zk_addr = getenv(env_key_zk_addr);

    zh = zookeeper_init(zk_addr, watcher, 10000, nullptr, nullptr, 0);
    if (!zh) {
        return errno;
    }

    std::string ip = zk::InetAddr::localhost();
    zk::BrokerNameAllocator brokerNameAllocator("/mq/brokerNames", "/mq", zh);


    std::cout << "BrokerName of local host: " << brokerNameAllocator.lookup(ip) << endl;
    std::string brokerName = brokerNameAllocator.acquire(ip, 2);
    std::cout << "Broker IP: " << ip << ", Broker Name: " << brokerName << std::endl;

    std::string ip2 = "127.0.0.1";
    std::string brokerName2 = brokerNameAllocator.acquire(ip2, 2);
    std::cout << "Broker IP2: " << ip2 << ", Broker Name: " << brokerName2 << std::endl;

    std::string ip3 = "127.0.0.2";
    std::string brokerName3 = brokerNameAllocator.acquire(ip3, 2);
    std::cout << "Broker IP2: " << ip3 << ", Broker Name: " << brokerName3 << std::endl;

    std::string ip4 = "127.0.0.3";
    std::string brokerName4 = brokerNameAllocator.acquire(ip4, 2);
    std::cout << "Broker IP2: " << ip4 << ", Broker Name: " << brokerName4 << std::endl;


    google::ShutdownGoogleLogging();

    return 0;
}