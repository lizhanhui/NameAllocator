#include "ZKClient.h"
namespace zk {
    ZKClient::ZKClient(const std::string &zk_connection_url, bool autoReconnect)
            : zk_address(zk_connection_url),
              connected(false),
              reconnectOnConnectionLoss(autoReconnect) {
        start();
    }

    void ZKClient::watcher(zhandle_t *handler, int type, int state, const char *path, void *watcherCtx) {
        if (ZOO_SESSION_EVENT == type) {
            ZKClient *zkClient = (ZKClient *)zoo_get_context(handler);

            if (ZOO_CONNECTED_STATE == state) {
                std::unique_lock<std::mutex> lock(zkClient->mtx);
                zkClient->connected = true;
                zkClient->cv.notify_all();
                spdlog::get("logger")->info("Connected");
            }

            if (ZOO_EXPIRED_SESSION_STATE == state) {
                spdlog::get("logger")->warn("Connection to ZK is lost");
                while (true) {
                    handler = zookeeper_init(zkClient->zk_address.c_str(), ZKClient::watcher, 10000, nullptr, zkClient, 0);
                    if (nullptr != handler) {
                        spdlog::get("logger")->info("Connect ZK OK");
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }
    }

    void ZKClient::start() {
        handler = zookeeper_init(zk_address.c_str(), ZKClient::watcher, 10000, nullptr, this, 0);
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]{return connected;});
    }

    bool ZKClient::mkdir(const std::string &path, const std::string& data, bool ephemeral) {
        validate_zk_connection();
        if (exists(path)) {
            return false;
        }

        int BUFFER_SIZE = 1024;
        char buf[BUFFER_SIZE];
        const char* pData = data.c_str();
        int status = zoo_create(handler, path.c_str(), pData, strlen(pData), &ZOO_OPEN_ACL_UNSAFE,
                                ephemeral ? ZOO_EPHEMERAL : 0, buf, sizeof(buf) - 1);
        return status == ZOK;
    }

    bool ZKClient::rm(const std::string &path) {
        validate_zk_connection();
        return ZOK == zoo_delete(handler, path.c_str(), -1);
    }

    void ZKClient::mkdirs(const std::string &path, const char *data) {
        validate_zk_connection();
        struct Stat stat;
        int rc = zoo_exists(handler, path.c_str(), 0, &stat);
        if (ZNONODE == rc) {
            const char *separator = "/";
            std::string::size_type pos = 1;
            pos = path.find_first_of(separator, pos + 1);
            while (true) {
                std::string parent = (pos == path.npos) ? path : path.substr(0, pos);
                if (ZNONODE == zoo_exists(handler, parent.c_str(), 0, &stat)) {
                    if (pos == path.npos && nullptr != data) {
                        rc = zoo_create(handler, parent.c_str(), data, strlen(data), &ZOO_OPEN_ACL_UNSAFE, 0, nullptr, 0);
                    } else {
                        rc = zoo_create(handler, parent.c_str(), nullptr, -1, &ZOO_OPEN_ACL_UNSAFE, 0, nullptr, 0);
                    }
                    if (rc) {
                        spdlog::get("logger")->error("Failed to create node: {}", parent);
                        break;
                    }
                    spdlog::get("logger")->info("New node: {} created OK", parent);
                }

                if (pos == path.npos) {
                    break;
                }

                pos = path.find_first_of(separator, pos + 1);
            }
        } else {
            spdlog::get("logger")->error("Path: {} has already existed", path);
        }
    }

    bool ZKClient::exists(const std::string &path) {
        validate_zk_connection();
        struct Stat stat;
        int status = zoo_exists(handler, path.c_str(), 0, &stat);
        return status == ZOK;
    }

    std::unordered_set<std::string> ZKClient::children(const std::string &path) {
        validate_zk_connection();
        struct String_vector vector;
        std::unordered_set<std::string> s;
        int status = zoo_get_children(handler, path.c_str(), 0, &vector);
        switch (status) {
            case ZOK:
                for (int i = 0; i < vector.count; ++i) {
                    s.insert(std::string(*(vector.data + i)));
                }
                return s;

            case ZNONODE:
                spdlog::get("logger")->error("{}  does not exist", path);
                break;

            default:
                spdlog::get("logger")->error("Unknown error. Error code: {}", status);
                break;
        }

        // Abort
        throw status;
    }

    const std::string ZKClient::get(const std::string &path) {
        validate_zk_connection();
        int BUFFER_SIZE = 4096;
        struct Stat stat;
        char buffer[BUFFER_SIZE];
        int status = zoo_get(handler, path.c_str(), 0, buffer, &BUFFER_SIZE, &stat);
        std::string result;
        switch (status) {
            case ZOK:
                if (BUFFER_SIZE > 0) {
                    result.append(buffer, BUFFER_SIZE);
                } else {
                    spdlog::get("logger")->info("{} has no data", path);
                }
                return result;

            case ZNONODE:
                spdlog::get("logger")->error("{} does not exist", path);
                break;

            default:
                spdlog::get("logger")->error("Unknown error. Error code: {}", status);
        }

        throw status;
    }

    bool ZKClient::set(const std::string &path, const std::string &data) {
        validate_zk_connection();
        int status = zoo_set(handler, path.c_str(), data.c_str(), data.size(), -1);
        if (ZOK == status) {
            return true;
        }
        spdlog::get("logger")->error("Failed to set data for path: {}", path);
        return false;
    }

    void ZKClient::validate_zk_connection() {
        if (nullptr == handler) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock);
        }
    }

    ZKClient::~ZKClient() {
        if (nullptr != handler) {
            zookeeper_close(handler);
            handler = nullptr;
        }
    }
}