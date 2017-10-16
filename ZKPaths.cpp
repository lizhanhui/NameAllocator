#include "ZKPaths.h"

namespace zk {

    bool ZKPaths::mkdir(zhandle_t *handler, const std::string &path, const std::string& data) {
        if (exists(handler, path)) {
            return false;
        }

        int BUFFER_SIZE = 1024;
        char buf[BUFFER_SIZE];
        const char* pData = data.c_str();
        int status = zoo_create(handler, path.c_str(), pData, strlen(pData), &ZOO_OPEN_ACL_UNSAFE, 0, buf,
                                sizeof(buf) - 1);
        return status == ZOK;
    }

    bool ZKPaths::rm(zhandle_t *handler, const std::string &path) {
        return ZOK == zoo_delete(handler, path.c_str(), -1);
    }

    void ZKPaths::mkdirs(zhandle_t *handler, const std::string &path) {
        struct Stat stat;
        int rc = zoo_exists(handler, path.c_str(), 0, &stat);
        if (ZNONODE == rc) {
            const char *separator = "/";
            std::string::size_type pos = 1;
            pos = path.find_first_of(separator, pos + 1);
            while (true) {
                std::string parent = (pos == path.npos) ? path : path.substr(0, pos);
                if (ZNONODE == zoo_exists(handler, parent.c_str(), 0, &stat)) {
                    rc = zoo_create(handler, parent.c_str(), nullptr, -1, &ZOO_OPEN_ACL_UNSAFE, 0, nullptr, 0);
                    if (rc) {
                        LOG(ERROR) << "Failed to create node: " << parent << std::endl;
                        break;
                    }
                    LOG(INFO) << "New node: " << parent << " created" << std::endl;
                }

                if (pos == path.npos) {
                    break;
                }

                pos = path.find_first_of(separator, pos + 1);
            }
        } else {
            LOG(ERROR) << "Path: " << path << " has already existed" << std::endl;
        }
    }

    bool ZKPaths::exists(zhandle_t *handler, const std::string &path) {
        struct Stat stat;
        int status = zoo_exists(handler, path.c_str(), 0, &stat);
        return status == ZOK;
    }

    std::unordered_set<std::string> ZKPaths::children(zhandle_t *handler, const std::string &path) {
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
                LOG(ERROR) << path << " does not exist";
                break;

            default:
                LOG(ERROR) << "Unknown error. Error code: " << status;
                break;
        }

        // Abort
        throw status;
    }

    const std::string ZKPaths::get(zhandle_t *handler, const std::string &path) {
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
                    LOG(INFO) << path << " has no data";
                }
                return result;

            case ZNONODE:
                LOG(ERROR) << path << " does not exist";
                break;

            default:
                LOG(ERROR) << "Unknown error. Error code: " << status;
        }

        throw status;
    }

    bool ZKPaths::set(zhandle_t *handler, const std::string &path, const std::string &data) {
        int status = zoo_set(handler, path.c_str(), data.c_str(), data.size(), -1);
        if (ZOK == status) {
            return true;
        }
        LOG(ERROR) << "Failed to set data for path: " << path;
        return false;
    }

}