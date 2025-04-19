#ifndef API_H
#define API_H

#include <string>
#include <vector>
#include "graph.h"

using namespace std;

class ApiClient {
private:
    string apiKey;
    string baseUrl;

    // Helper method to make API requests
    string makeRequest(const string& url);

public:
    ApiClient(const string& key);
    ~ApiClient();

    // Search for a person by name and return a vector of possible matches (id, display name)
    vector<pair<int, string>> searchPerson(const string& name);

    // Get actor details by ID
    Actor* getPersonDetails(int personId);

    // Get movie details by ID
    Movie* getMovieDetails(int movieId, vector<int>& castIds, Graph& graph);

    // Build the graph, targeting a specific actor
    Graph* buildGraph(const vector<int>& actorIds);

    // Get actor object
    Actor getActorObject(const string& actorName);
};

#endif