#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <stack>
#include <set>
#include <chrono>
#include <functional>

#include "data.h"

using namespace std;

//basic structures:

//connection structure to represent a connection/link/edge between actors
struct Connection {
    int actorId;
    vector<Movie*> movies;

    Connection(int actorId) : actorId(actorId) {}
    ~Connection() {}
};

//TODO
// Path step structure to represent a step in the path between actors
struct PathStep {
    Actor* actor;
    Actor* prevActor;
    Movie* movie;

    PathStep(Actor* actor, Actor* prevActor = nullptr, Movie* movie = nullptr)
        : actor(actor), prevActor(prevActor), movie(movie) {}
};

//TODO
// Search result structure to store the result of a path search
struct SearchResult {
    vector<PathStep> path;
    double time_ms;
    int visited;

    SearchResult() : time_ms(0), visited(0) {}
};

class Graph {
private:

    //variables
    unordered_map<int, Actor*> actors; //(actor.id, actor.name)
    unordered_map<int, Movie*> movies; //(movie.id, movie.title)
    unordered_map<int, vector<Connection>> adjacencyList; //(actor.id, edges)
    api API;

    //not used yet
    //vector<Movie*> movies;
    //Data data;


    //helper functions:

    //adds actor to the graph structure (assumes actor is not already in the graph)
    void addNode(Actor* actor);

    //checks if there is a shared movie between two actors
    //if a shared movie is found, adds the connection/edge to the graph and breaks out of loop
    void findSharedMovie(Actor* actor, int targetActorID);

public:
    Graph() : API("07663db07b6982f498aef71b6b0997f7"){} //constructor

    //adds actor to the graph given a pointer to that actor object and the id of actor we want to connect it with
    void addActor(Actor* actor, int targetActorID);

    //adds movie to movie map and returns a pointer to it
    Movie* addMovie(int id, const string& title, const string& release_date, const string& poster_path);

    /////////////////////////////////////////////
    ///TODO:

    ~Graph() {
        // Delete all actors
        for (auto& pair : actors) {
            delete pair.second;
        }

        // TODO: Delete all movies
        // for (auto movie : movies) {
        //     delete movie;
        // }
    }


    void addMovie(Movie* movie);

    // Add a connection between actors through a movie
    bool addConnection(int actorId1, int actorId2, Movie* movie);

    // Get actor by ID
    Actor* getActor(int actorId) const;

    // BFS to find path between two actors
    SearchResult findPathBFS(int startActorId, int endActorId);

    // DFS to find path between two actors
    SearchResult findPathDFS(int startActorId, int endActorId);

    // Get graph statistics
    pair<int, int> getStats() const;
};

#endif // GRAPH_H
