#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <set>
#include <chrono>
#include <functional>

#include "api.h"

using namespace std;

//basic structures:

//stores all possible edges/connections of an actor/vertix
struct Connection {
    int actorId;
    vector<Movie*> movies;

    Connection(int actorId) : actorId(actorId) {}
    ~Connection() {}
};

//represents a step in the path between actors (actor -> movie -> actor)
struct PathStep {
    Actor* actor;
    Actor* prevActor;
    Movie* movie;

    PathStep(Actor* actor, Actor* prevActor = nullptr, Movie* movie = nullptr)
        : actor(actor), prevActor(prevActor), movie(movie) {}
};

//stores the result of a path search
struct SearchResult {
    vector<PathStep> path;
    double time_ms;
    int visited;
    double data_fetch_ms; //time spent fetching the data
    double algorithm_ms; //time spent in the algorithm itself

    SearchResult() : time_ms(0), visited(0) {}
};

class Graph {
private:

    //variables
    unordered_map<int, Actor*> actors; //(actor.id, actor.name)
    unordered_map<int, Movie*> movies; //(movie.id, movie.title)
    unordered_map<int, vector<Connection>> adjacencyList; //(actor.id, edges)
    api& API;

    //helper functions:

    //adds actor to the graph structure (assumes actor is not already in the graph)
    void addNode(Actor* actor);

    //checks if there is a shared movie between two actors
    //if a shared movie is found, adds the connection/edge to the graph and breaks out of loop
    void findSharedMovie(Actor* actor, int targetActorID);

    //add a connection between actors through a movie
    //if a connection is successful returns true, if it already exits returns false
    bool addConnection(int actorId1, int actorId2, Movie* movie);

    //expands the graph from a given actor by:
    // - fetching all their movies
    // - looking at the cast for each of those movies
    // - adding the ones who are not on graph already and connecting them to original actor
    void expandFromActor(int actorID, set<int>& targetSet);

    //TODO:
    // bool processNeighbors(int currentId, unordered_map<int, pair<int, Movie*>>& previous,
    //                        set<int>& visited, queue<int>& q, set<int>& otherVisited,
    //                        int& meetingPoint);

public:
    Graph(api& apiInstance) : API(apiInstance){} //constructor

    ~Graph() { //destructor
        //delete actors
        for (auto& pair : actors)
            delete pair.second;
        //delete movies
        for (auto& pair : movies)
            delete pair.second;
    }

    //adds actor to the graph given a pointer to that actor object and the id of actor we want to connect it with
    void addActor(Actor* actor, int targetActorID);

    //adds movie to movie map and returns a pointer to it
    Movie* addMovie(int id, const string& title, const string& release_date, const string& poster_path);

    //performs a breadth-first search on the graph to find path between two actors
    SearchResult findPathBFS(int startActorId, int endActorId);

    //returns the graph statistics -> in the pair:
    // - first value: total number of actors (vertices) in graph
    // - second value: total number of edges between actors
    pair<int, int> getStats() const;

    // TODO:
    // SearchResult findPathBidirectional(int startActorId, int endActorId);

};

#endif // GRAPH_H
