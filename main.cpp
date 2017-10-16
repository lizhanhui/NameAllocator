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

    const string s = "/Users/lizhanhui/test.properties";
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

    const char* path = "/xyz";
    const char* value = "value";

    std::cout << zk::ZKPaths::get(zh, string(path)) << std::endl;

    unordered_set<string> children = zk::ZKPaths::children(zh, string(path));
    for (auto next = children.begin(); next != children.end(); next++) {
        std::cout << *next << std::endl;
    }

    const char *non_existing_path = "/xyz/abc";

    zk::ZKPaths::mkdirs(zh, non_existing_path);

    zoo_set(zh, path, value, strlen(value), 0);

    std::cout << zk::ZKPaths::exists(zh, path) << std::endl;

    int rc = zoo_create(zh, path, value, strlen(value), &ZOO_OPEN_ACL_UNSAFE, 0, buffer, sizeof(buffer) - 1);

    if (rc) {
        cerr << "Failed to create ephemeral node. Error Code: " << rc << endl;
    } else {
        cout << "Node created" << endl;
    }

    int buffer_length = sizeof(buffer);
    struct Stat stat;
    rc = zoo_get(zh, path, 0, buffer, &buffer_length, &stat);

    if (rc) {
        fprintf(stderr, "Error Code: %d\n", rc);
    } else {
        fprintf(stdout, "Value: %s\n", buffer);
        cout << "Children Number: " << stat.numChildren << endl;
    }

    zk::ZKPaths::mkdirs(zh, "/mq/brokerNames/broker-a/a");


    zk::BrokerNameAllocator brokerNameAllocator("/mq/brokerNames", zh);


    std::string ip = zk::InetAddr::localhost();
    std::cout << "BrokerName of local host: " << brokerNameAllocator.lookup(ip) << endl;
    std::string brokerName = brokerNameAllocator.acquire(ip, 2);

    std::cout << "Broker IP: " << ip << ", Broker Name: " << brokerName << std::endl;


    zookeeper_close(zh);
    google::ShutdownGoogleLogging();

    return 0;
}