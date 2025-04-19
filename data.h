#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include "json.hpp"

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

class Data {
private:
    string api_key;
    unordered_map<string, int> actorID;
    unordered_map<int, Actor> actorObjects;

    //helper function: encodes spaces in actor names for the URL
    //e.g. "Tom Hanks" -> "Tom%20Hanks"
    string urlEncode(string& str);

    // Helper function to fetch data using libcurl
    string fetchData(const string& url);

public:
    //constructor
    Data(string key) : api_key(key){}

    Actor getActorObject(string actor_name); //returns the object for the given actor

    //both functions return an empty vector {} in case the actor/movie was not found
    vector<Movie> getMovies(string actor_name); //returns a list of movies that have a given actor
    vector<Actor> getActors(string movie_name); //returns a list of actors in a given movie

    // Static method for URL encoding
    static string urlEncode(const string& str);
};

#endif //DATA_H