#ifndef ZOOKEEPER_PROPERTIES_H
#define ZOOKEEPER_PROPERTIES_H

#include <unordered_map>
#include <string>
#include <fstream>
#include <glog/logging.h>

namespace zk {
    class Properties {
    public:
        Properties() {

        }

        virtual ~Properties();

        /**
         * Load properties file into map.
         * @param file Path to the properties file.
         */
        void load(const std::string &file);

        /**
         * Test if the given key exists or not.
         * @param key Key to test
         * @return true if the key exists; false otherwise.
         */
        bool has(const std::string& key);

        /**
         * Get value of the given key.
         * @param key
         * @return Value of the key.
         */
        std::string get(const std::string& key);

    private:
        std::unordered_map<std::string, std::string> data;

    };
}

#endif //ZOOKEEPER_PROPERTIES_H
