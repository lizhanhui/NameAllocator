#include "ZKPaths.h"

void zk::ZKPaths::mkdirs(zhandle_t *handler, const std::string &path) {
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

bool zk::ZKPaths::exists(zhandle_t *handler, const std::string &path) {
    return ZOK == zoo_exists(handler, path.c_str(), 0, nullptr);
}