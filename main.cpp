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
    google::SetLogDestination(google::GLOG_INFO, "/Users/lizhanhui/logs/zk_");

    zk::BrokerNameAllocator brokerNameAllocator("/mq/brokerNames");

    std::string ip = zk::InetAddr::localhost();
    std::string brokerName = brokerNameAllocator.acquire(ip, 2);

    std::cout << "Broker IP: " << ip << ", Broker Name: " << brokerName << std::endl;

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

    const char *non_existing_path = "/xyz/abc";

    zk::ZKPaths::mkdirs(zh, non_existing_path);

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

    zookeeper_close(zh);
    google::ShutdownGoogleLogging();

    return 0;
}