//
// Created by gioru on 4/20/2025.
//

#ifndef API_H
#define API_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class api {
private:
    string api_key;
    string base_url;

public:
    api(const string& key) : api_key(key), base_url("https://api.themoviedb.org/3") {} //constructor

    string fetchData(const string &url);
    string urlEncode(const string &str);
};



#endif //API_H
