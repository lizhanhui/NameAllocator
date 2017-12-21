#ifndef NAME_ALLOCATOR_ZK_CLIENT_H
#define NAME_ALLOCATOR_ZK_CLIENT_H

#include <unordered_set>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <zookeeper/zookeeper.h>
#include "spdlog/spdlog.h"

namespace zk {

    class ZKClient {
    public:
        ZKClient(const std::string& zk_connection_url, bool autoReconnect = true);

        bool mkdir(const std::string &path, const std::string& data, bool ephemeral = false);

        bool rm(const std::string &path);

        void mkdirs(const std::string& path, const char* data);

        bool exists(const std::string& path);

        std::unordered_set<std::string> children(const std::string& path);

        const std::string get(const std::string& path);

        bool set(const std::string &path, const std::string &data);

        static void watcher(zhandle_t* handler, int type, int state, const char* path, void* watcherCtx);

        virtual ~ZKClient();

    private:
        std::string zk_address;
        zhandle_t* handler;
        std::mutex mtx;
        std::condition_variable cv;
        bool connected;
        bool reconnectOnConnectionLoss;

        void start();

        void validate_zk_connection();
    };
}

#endif //NAME_ALLOCATOR_ZK_CLIENT_H
