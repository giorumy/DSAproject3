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

using namespace std;

//basic structure

// Movie structure to store movie information
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

// Connection structure to represent a connection between actors
struct Connection {
    int actorId;
    vector<Movie*> movies;

    Connection(int actorId) : actorId(actorId) {}
    ~Connection() {
        // We don't delete movies here as they are owned by the Graph class
    }
};

// Path step structure to represent a step in the path between actors
struct PathStep {
    Actor* actor;
    Actor* prevActor;
    Movie* movie;

    PathStep(Actor* actor, Actor* prevActor = nullptr, Movie* movie = nullptr)
        : actor(actor), prevActor(prevActor), movie(movie) {}
};

// Search result structure to store the result of a path search
struct SearchResult {
    vector<PathStep> path;
    double time_ms;
    int visited;

    SearchResult() : time_ms(0), visited(0) {}
};

class Graph {
private:
    unordered_map<int, Actor*> actors;
    unordered_map<int, vector<Connection>> adjacencyList;
    vector<Movie*> movies; // Store all movies to manage memory

public:
    Graph() {}

    // Destructor to clean up memory
    ~Graph() {
        // Delete all actors
        for (auto& pair : actors) {
            delete pair.second;
        }

        // Delete all movies
        for (auto movie : movies) {
            delete movie;
        }
    }

    // Add an actor to the graph
    void addActor(Actor* actor);

    // Add a movie to the graph
    Movie* addMovie(int id, const string& title, const string& release_date);

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
