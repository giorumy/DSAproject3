#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct Movie {
    int id;
    string title;
    string release_date;
    string poster_path;

    Movie(int id, string title, string release_date, string poster_path)
        : id(id), title(title), release_date(release_date), poster_path(poster_path) {}
};

// Actor structure to store actor information
struct Actor {
    int id;
    string name;
    string profile_path;

    Actor() : id(0) {}
    Actor(int id, string name, string profile_path = "")
        : id(id), name(name), profile_path(profile_path) {}
};

class Data {
private:
    string api_key;

    //helper function: encodes spaces in actor names for the URL
    //e.g. "Tom Hanks" -> "Tom%20Hanks"
    string urlEncode(string& str);
public:
    //constructor
    Data(string key) : api_key(key){}

    //both functions return an empty vector {} in case the actor/movie was not found
    vector<Movie> getMovies(string actor_name); //returns a list of movies that have a given actor
    vector<Actor> getActors(string movie_name); //returns a list of actors in a given movie
};

#endif //DATA_H