#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <vector>
#include <string>
using namespace std;

//I am not sure what would be the best way of representing the actors and movies.
//We could do the map<name, index> way, the vector<Actor>, or any other that you guys think would work best
struct Actor {
    string name;
    string picture;
    vector<string> movies; //maybe? then go through these movies or something idk
};



#endif //DATA_H
