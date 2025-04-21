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

#include "api.h"

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
    bool processNeighbors(int currentActorId, unordered_map<int, pair<int, Movie*>>& previous, set<int>& visited, queue<int>& q,
        set<int>& otherVisited, int& meetingPoint);

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

    //expands the graph from a given actor by:
    // - fetching all their movies
    // - looking at the cast for each of those movies
    // - adding the ones who are not on graph already and connecting them to original actor
    void expandFromActor(int actorID, set<int>& targetSet);
    // Get actor by ID
    Actor* getActor(int actorId) const;

    // BFS to find path between two actors
    SearchResult findPathBFS(int startActorId, int endActorId);

    // Bidirectional Search from each end
    SearchResult findPathBDS(int startActorId, int endActorId);

    // Get graph statistics
    pair<int, int> getStats() const;
};

#endif // GRAPH_H
