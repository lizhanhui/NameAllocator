#ifndef ZOOKEEPER_ZKUTILS_H
#define ZOOKEEPER_ZKUTILS_H

#include <string>
#include <vector>
#include <zookeeper/zookeeper.h>
#include <glog/logging.h>
#include <unordered_set>

namespace zk {
    class ZKPaths {
    public:
        static bool mkdir(zhandle_t *handler, const std::string &path, const std::string& data);

        static bool rm(zhandle_t *handler, const std::string &path);

        static void mkdirs(zhandle_t *handler, const std::string& path);

        static bool exists(zhandle_t *handler, const std::string& path);

        static std::unordered_set<std::string> children(zhandle_t *handler, const std::string& path);

        static const std::string get(zhandle_t* handler, const std::string& path);


    private:
    };
}

#endif //ZOOKEEPER_ZKUTILS_H
