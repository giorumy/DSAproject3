#include "api.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

ApiClient::ApiClient(const string& apiKey) : apiKey(apiKey), baseUrl("https://api.themoviedb.org/3") {
    // No initialization needed
}

ApiClient::~ApiClient() {
    // No cleanup needed
}

string ApiClient::makeRequest(const string& url) {
    // Create a command to fetch data using curl and save to a temporary file
    string tempFile = "temp_response.json";
    string command = "curl -s \"" + url + "\" > " + tempFile;

    // Execute the command
    int result = system(command.c_str());

    if (result != 0) {
        cerr << "Failed to execute command: " << command << endl;
        return "";
    }

    // Read the file
    ifstream file(tempFile);
    if (!file.is_open()) {
        cerr << "Failed to open temporary file: " << tempFile << endl;
        return "";
    }

    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // Remove the temporary file
    remove(tempFile.c_str());

    return content;
}

vector<pair<int, string>> ApiClient::searchPerson(const string& name) {
    vector<pair<int, string>> results;
    string encodedName = Data::urlEncode(name);
    string url = baseUrl + "/search/person?api_key=" + apiKey + "&query=" + encodedName;

    string response = makeRequest(url);

    if (!response.empty()) {
        json data = json::parse(response);
        if (data.contains("results") && data["results"].is_array()) {
            for (const auto& result : data["results"]) {
                if (result.contains("id") && result.contains("name")) {
                    results.push_back(make_pair(result["id"].get<int>(), result["name"].get<string>()));
                }
            }
        }
    }

    return results;
}

Actor* ApiClient::getPersonDetails(int personId) {
    stringstream ss;
    ss << baseUrl << "/person/" << personId << "?api_key=" << apiKey;

    string response = makeRequest(ss.str());
    if (response.empty()) {
        return nullptr;
    }

    try {
        json data = json::parse(response);

        if (data.contains("id") && data.contains("name")) {
            int id = data["id"];
            string name = data["name"];
            string profile_path = data.contains("profile_path") && !data["profile_path"].is_null() ?
                                 data["profile_path"].get<string>() : "";

            return new Actor(id, name, profile_path);
        }
    } catch (const exception& e) {
        cerr << "Error parsing JSON: " << e.what() << endl;
    }

    return nullptr;
}

Movie* ApiClient::getMovieDetails(int movieId, vector<int>& castIds, Graph& graph) {
    stringstream ss;
    ss << baseUrl << "/movie/" << movieId << "?api_key=" << apiKey << "&append_to_response=credits";

    string response = makeRequest(ss.str());
    if (response.empty()) {
        return nullptr;
    }

    try {
        json data = json::parse(response);

        if (data.contains("id") && data.contains("title")) {
            int id = data["id"];
            string title = data["title"];
            string release_date = data.contains("release_date") && !data["release_date"].is_null() ?
                                 data["release_date"].get<string>() : "";
            string poster_path = data.contains("poster_path") && !data["poster_path"].is_null() ?
                                data["poster_path"].get<string>() : "";

            Movie* movie = graph.addMovie(id, title, release_date, poster_path);

            // Extract cast IDs
            if (data.contains("credits") && data["credits"].contains("cast") &&
                data["credits"]["cast"].is_array()) {

                // Sort by order and take top 5
                vector<pair<int, int>> castWithOrder;
                for (const auto& cast : data["credits"]["cast"]) {
                    if (cast.contains("id") && cast.contains("order")) {
                        castWithOrder.push_back(make_pair(cast["id"], cast["order"]));
                    }
                }

                sort(castWithOrder.begin(), castWithOrder.end(),
                     [](const pair<int, int>& a, const pair<int, int>& b) {
                         return a.second < b.second;
                     });

                for (size_t i = 0; i < min(castWithOrder.size(), size_t(5)); i++) {
                    castIds.push_back(castWithOrder[i].first);
                }
            }

            return movie;
        }
    } catch (const exception& e) {
        cerr << "Error parsing JSON: " << e.what() << endl;
    }

    return nullptr;
}

Graph* ApiClient::buildGraph(const vector<int>& actorIds) {
    Graph* graph = new Graph();

    // Add actors to the graph
    for (size_t i = 0; i < actorIds.size(); i++) {
        int actorId = actorIds[i];
        int targetId = (i + 1 < actorIds.size()) ? actorIds[i + 1] : actorIds[0];

        Actor* actor = getPersonDetails(actorId);
        if (actor) {
            graph->addActor(actor, targetId);
        }
    }

    return graph;
}

Actor ApiClient::getActorObject(const string& actorName) {
    Data data(apiKey);
    return data.getActorObject(actorName);
}