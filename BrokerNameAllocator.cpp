#include <InetAddr.h>
#include <unordered_set>
#include <ZKPaths.h>
#include "BrokerNameAllocator.h"

namespace zk {

    std::string BrokerNameAllocator::lookup(const std::string &ip) {
        std::unordered_set<std::string> broker_name_set = ZKPaths::children(handler, prefix);
        if (!broker_name_set.empty()) {
            for (const std::string &broker_name : broker_name_set) {
                std::string name_node = prefix + "/" + broker_name;
                std::unordered_set<std::string> ip_set = ZKPaths::children(handler, name_node);
                for (const std::string &ip_node : ip_set) {
                    if (ip == ZKPaths::get(handler, name_node + "/" + ip_node)) {
                        return broker_name;
                    }
                }
            }
        }

        throw -1;
    }

    std::string BrokerNameAllocator::acquire(const std::string &ip, int span) {


        return "OK";
    }

    bool BrokerNameAllocator::release(const std::string &broker_name, const std::string &ip) {
        return true;
    }
}
