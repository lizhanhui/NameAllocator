#ifndef ZOOKEEPER_INETADDR_H
#define ZOOKEEPER_INETADDR_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include <string>
#include <glog/logging.h>

namespace zk {
    class InetAddr {
    public:

        /**
         * Get standard dot notation of IPv4 address of local host.
         * @return IP address of local host.
         */
        static std::string localhost();

    };
}

#endif //ZOOKEEPER_INETADDR_H
