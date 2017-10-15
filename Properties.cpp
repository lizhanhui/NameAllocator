#include "Properties.h"

namespace zk {
    void Properties::load(const std::string &file) {
        std::fstream f(file.c_str(), std::ios_base::in );
        for(std::string s; std::getline(f, s); ) {
            std::string::size_type pos = s.find('=');
            if (pos != std::string::npos) {
                std::string key = s.substr(0, pos);
                std::string value = s.substr(pos + 1);
                data.insert(std::make_pair(key, value));
                LOG(INFO) << "Key: " << key << ", Value: " << value;
            }
        }
    }

    bool Properties::has(const std::string &key) {
        return data.find(key) != data.end();
    }

    std::string Properties::get(const std::string &key) {
        auto search = data.find(key);
        if (search != data.end()) {
            return search->second;
        }
        LOG(WARNING) << "Key: " << key << " is not existing";
        throw std::exception();
    }

    Properties::~Properties() {

    }
}