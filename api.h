#ifndef API_H
#define API_H

#include <iostream>
#include <fstream>
#include <optional> //https://en.cppreference.com/w/cpp/utility/optional
#include "json.hpp"

#include <cstdlib>
#include <sstream>
#include <unordered_map>

using json = nlohmann::json;
using namespace std;

//movie object to store actor information including id (for url), name, release date (year/month/day) and poster path (for image url)
struct Movie {
    int id;
    string title;
    string release_date;
    string poster_path;

    Movie(int id, string title, string release_date, string poster_path)
        : id(id), title(title), release_date(release_date), poster_path(poster_path) {}
};

//actor object to store actor information including id (for url), name, and profile path (for image url)
struct Actor {
    int id;
    string name;
    string profile_path;

    Actor() : id(0) {}
    Actor(int id, string name, string profile_path = "")
        : id(id), name(name), profile_path(profile_path) {}
};

class api {
private:
    string api_key;
    string base_url;

public:
    api(const string& key) : api_key(key), base_url("https://api.themoviedb.org/3") {} //constructor

    //TODO: maybe these two methods could be private?
    string fetchData(const string &url); //makes API requests
    string urlEncode(const string &str); //encodes spaces in actor names for the URL - e.g. "Tom Hanks" => "Tom%20Hanks"

    int searchActor(const string& name); //returns id of first actor in api search - e.g. "Tom Hanks" => 31
    Actor* getActor(int actorID); //returns pointer to actor object given their id

    vector<Actor> getActors(const string& movieName);
    vector<Movie> getMovies(const string& actorName);
};



#endif //API_H
