#include "InetAddr.h"

namespace zk {
    std::string InetAddr::localhost() {
        struct ifaddrs *if_address = nullptr;
        int status = getifaddrs(&if_address);
        if (status == -1) {
            LOG(ERROR) << "Failed to call getifaddrs(). Error message: " << gai_strerror(errno);
            throw std::exception();
        }

        char buf[INET_ADDRSTRLEN];
        std::string ip;

        for (struct ifaddrs *ifa = if_address; ifa != nullptr; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) {
                continue;
            }

            if (ifa->ifa_addr->sa_family == AF_INET) {
                struct in_addr *tmp = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, tmp, buf, INET_ADDRSTRLEN);
                ip = std::string(buf);
                if (ip != "127.0.0.1") {
                    break;
                }
            }
        }

        if (if_address) {
            freeifaddrs(if_address);
        }

        return ip;
    }
}