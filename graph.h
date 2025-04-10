#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

//I am not sure what would be the best way of representing the actors and movies.
//We could do the map<name, index> way, the vector<Actor>, or any other that you guys think would work best
struct Actor {
  string name;
  string picture;
  vector<string> movies; //maybe? then go through these movies or something idk
};

class graph {
  //this will be the adjacency list connecting the actors?

  private:

    //unordered map would be best because we don't need to sort this data
    unordered_map<int, vector<int>> adjacencyList; //or unordered_map<Actor, vector<Actor>>

  public:

    //function to add a new item to the map
    //function to perform searches
};



#endif //GRAPH_H
