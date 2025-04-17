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

    Movie(int id, string title, string release_date)
        : id(id), title(title), release_date(release_date) {}
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

    vector<Movie> getMovies(string actor_name); //returns a list of movies that have a given actor
    vector<Actor> getActors(string movie_name); //returns a list of actors in a given movie

    //TODO: the functions below are for if we end up having time for front end stuff.
    string getActorImage(string actorName); //returns url of actor image
    string getMovieImage(string movieName); //returns url of movie poster image
};

#endif //DATA_H