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

    //TODO: maybe these two methods could be private?
    string fetchData(const string &url); //makes API requests
    string urlEncode(const string &str); //encodes spaces in actor names for the URL - e.g. "Tom Hanks" => "Tom%20Hanks"

    int searchActor(const string& name); //returns id of first actor in api search
};



#endif //API_H
