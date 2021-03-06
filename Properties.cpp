#include "Properties.h"

namespace zk {
    void Properties::load(const std::string &file) {
        std::fstream f(file.c_str(), std::ios_base::in );
        for(std::string s; std::getline(f, s); ) {
            if (!s.empty() && s[0] == '#') {
                continue;
            }

            std::string::size_type pos = s.find('=');
            if (pos != std::string::npos) {
                std::string key = trim(s.substr(0, pos));
                std::string value = trim(s.substr(pos + 1));
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

    std::string Properties::trim(const std::string &s) {

        if (s.find(' ') == std::string::npos) {
            return s;
        }

        std::string::size_type l = s.find_first_not_of(' ');
        std::string::size_type r = s.find_last_not_of(' ');
        return s.substr(l, r - l + 1);
    }
}