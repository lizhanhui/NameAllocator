#ifndef ZOOKEEPER_ZKUTILS_H
#define ZOOKEEPER_ZKUTILS_H

#include <string>
#include <zookeeper/zookeeper.h>
#include <glog/logging.h>

namespace zk {
    class ZKPaths {
    public:

        static void mkdirs(zhandle_t *handler, const std::string& path);

        static bool exists(zhandle_t *handler, const std::string& path);

    private:
    };
}

#endif //ZOOKEEPER_ZKUTILS_H
