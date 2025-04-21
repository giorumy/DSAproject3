#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include "graph.h"
#include "api.h"

using namespace std;

// Function to display the path in a linear format: actor1 - movie - actor2 - movie - actor3
void displayPath(const SearchResult& result, const string& algorithm, ostream& out = cout) {
    if (result.path.empty()) {
        out << "No path found using " << algorithm << "." << endl;
        return;
    }

    out << "\n===== Path found using " << algorithm << " =====" << endl;
    out << "Total time: " << fixed << setprecision(2) << result.time_ms << " ms" << endl;
    out << "Data fetch time: " << fixed << setprecision(2) << result.data_fetch_ms << " ms" << endl;
    out << "Algorithm time: " << fixed << setprecision(2) << result.algorithm_ms << " ms" << endl;
    out << "Nodes visited: " << result.visited << endl;
    out << "\nPath: ";

    // First, print the detailed path in the original format for debugging
    out << "\nDetailed path:" << endl;
    for (size_t i = 0; i < result.path.size(); i++) {
        const auto& step = result.path[i];
        out << i + 1 << ". " << step.actor->name << " (ID: " << step.actor->id << ")" << endl;

        if (i < result.path.size() - 1) {
            // Find the next step to get the movie connection
            const auto& nextStep = result.path[i + 1];
            Movie* movie = nullptr;

            // If this step has a movie, use it
            if (step.movie) {
                movie = step.movie;
                out << "   ↓ appeared in \"" << movie->title << "\" ("
                    << (movie->release_date.empty() ? "N/A" : movie->release_date.substr(0, 4))
                    << ") with" << endl;
            }
            // If next step has a prevActor that matches this actor, use its movie
            else if (nextStep.prevActor && nextStep.prevActor->id == step.actor->id && nextStep.movie) {
                movie = nextStep.movie;
                out << "   ↓ appeared in \"" << movie->title << "\" ("
                    << (movie->release_date.empty() ? "N/A" : movie->release_date.substr(0, 4))
                    << ") with" << endl;
            }
            else {
                out << "   ↓ connection details missing" << endl;
            }
        }
    }

    // Now print the linear format
    out << "\nLinear path: ";
    for (size_t i = 0; i < result.path.size(); i++) {
        const auto& step = result.path[i];

        // Print actor name
        out << step.actor->name;

        // If not the last actor, print the movie connection
        if (i < result.path.size() - 1) {
            Movie* movie = nullptr;

            // If this step has a movie, use it
            if (step.movie) {
                movie = step.movie;
            }
            // If next step has a prevActor that matches this actor, use its movie
            else if (i+1 < result.path.size() &&
                     result.path[i+1].prevActor &&
                     result.path[i+1].prevActor->id == step.actor->id &&
                     result.path[i+1].movie) {
                movie = result.path[i+1].movie;
            }

            if (movie) {
                string year = movie->release_date.empty() ? "N/A" : movie->release_date.substr(0, 4);
                out << " → [" << movie->title << " (" << year << ")] → ";
            } else {
                out << " → [Unknown Movie] → ";
            }
        }
    }
    out << endl;
}

int main() {
    cout << "=== StarPath: Find connections between actors ===\n" << endl;

    // Open output file
    ofstream outputFile("starpath_results.txt");

    // Using the API key from Graph constructor
    string apiKey = "07663db07b6982f498aef71b6b0997f7";
    ApiClient apiClient(apiKey);

    // Get first actor
    string actor1Name;
    cout << "Enter the first actor name: ";
    getline(cin, actor1Name);

    auto actor1Results = apiClient.searchPerson(actor1Name);
    if (actor1Results.empty()) {
        cout << "No actors found with that name." << endl;
        return 1;
    }

    cout << "\nResults for first actor:" << endl;
    for (size_t i = 0; i < actor1Results.size() && i < 5; i++) {
        cout << i + 1 << ". " << actor1Results[i].second << endl;
    }

    int actor1Index;
    cout << "\nSelect actor number: ";
    cin >> actor1Index;
    cin.ignore(); // Clear the newline

    if (actor1Index < 1 || actor1Index > static_cast<int>(actor1Results.size())) {
        cout << "Invalid selection." << endl;
        return 1;
    }

    int actor1Id = actor1Results[actor1Index - 1].first;
    string actor1DisplayName = actor1Results[actor1Index - 1].second;

    // Get second actor
    string actor2Name;
    cout << "\nEnter the second actor name: ";
    getline(cin, actor2Name);

    auto actor2Results = apiClient.searchPerson(actor2Name);
    if (actor2Results.empty()) {
        cout << "No actors found with that name." << endl;
        return 1;
    }

    cout << "\nResults for second actor:" << endl;
    for (size_t i = 0; i < actor2Results.size() && i < 5; i++) {
        cout << i + 1 << ". " << actor2Results[i].second << endl;
    }

    int actor2Index;
    cout << "\nSelect actor number: ";
    cin >> actor2Index;
    cin.ignore(); // Clear the newline

    if (actor2Index < 1 || actor2Index > static_cast<int>(actor2Results.size())) {
        cout << "Invalid selection." << endl;
        return 1;
    }

    int actor2Id = actor2Results[actor2Index - 1].first;
    string actor2DisplayName = actor2Results[actor2Index - 1].second;

    // Build the graph
    cout << "\nBuilding graph for " << actor1DisplayName << " and " << actor2DisplayName << "..." << endl;
    cout << "This may take a moment as we fetch data from TMDB API..." << endl;

    Graph* graph = new Graph();

    // Get actor details
    Actor* actor1 = apiClient.getPersonDetails(actor1Id);
    Actor* actor2 = apiClient.getPersonDetails(actor2Id);

    if (!actor1 || !actor2) {
        cout << "Failed to get actor details." << endl;
        delete graph;
        return 1;
    }

    // Add actors to graph with targeted approach
    cout << "Adding " << actor1DisplayName << " to graph..." << endl;
    graph->addActor(actor1, actor2Id);

    cout << "Adding " << actor2DisplayName << " to graph..." << endl;
    graph->addActor(actor2, actor1Id);

    auto stats = graph->getStats();
    cout << "\nGraph built with " << stats.first << " actors and " << stats.second << " connections." << endl;

    // Find paths using both algorithms
    cout << "\nFinding path using Bidirectional Search from " << actor1DisplayName << " to " << actor2DisplayName << "..." << endl;
    auto biResult = graph->findPathBidirectional(actor1Id, actor2Id);

    cout << "\nFinding path using BFS from " << actor1DisplayName << " to " << actor2DisplayName << "..." << endl;
    auto bfsResult = graph->findPathBFS(actor1Id, actor2Id);

    // Display results to console
    displayPath(biResult, "Bidirectional Search");
    displayPath(bfsResult, "BFS");

    // Write results to file if file is open
    if (outputFile.is_open()) {
        outputFile << "=== StarPath Results: " << actor1DisplayName << " to " << actor2DisplayName << " ===" << endl;
        displayPath(biResult, "Bidirectional Search", outputFile);
        displayPath(bfsResult, "BFS", outputFile);

        // Write performance comparison to file
        outputFile << "\n===== Algorithm Performance Comparison =====" << endl;
        outputFile << "┌─────────────────┬────────────┬────────────┬────────────┬───────────┬────────────┐" << endl;
        outputFile << "│ Algorithm       │ Total Time │ Data Fetch │ Algorithm  │ Nodes     │ Path       │" << endl;
        outputFile << "│                 │ (ms)       │ Time (ms)  │ Time (ms)  │ Visited   │ Length     │" << endl;
        outputFile << "├─────────────────┼────────────┼────────────┼────────────┼───────────┼────────────┤" << endl;

        outputFile << "│ Bidirectional   │ " << setw(10) << fixed << setprecision(2) << biResult.time_ms
             << " │ " << setw(10) << biResult.data_fetch_ms
             << " │ " << setw(10) << biResult.algorithm_ms
             << " │ " << setw(9) << biResult.visited
             << " │ " << setw(10) << (biResult.path.empty() ? 0 : biResult.path.size()) << " │" << endl;

        outputFile << "│ BFS             │ " << setw(10) << fixed << setprecision(2) << bfsResult.time_ms
             << " │ " << setw(10) << bfsResult.data_fetch_ms
             << " │ " << setw(10) << bfsResult.algorithm_ms
             << " │ " << setw(9) << bfsResult.visited
             << " │ " << setw(10) << (bfsResult.path.empty() ? 0 : bfsResult.path.size()) << " │" << endl;

        outputFile << "└─────────────────┴────────────┴────────────┴────────────┴───────────┴────────────┘" << endl;

        cout << "\nResults have been saved to starpath_results.txt" << endl;
    }

    // Compare algorithms
    cout << "\n===== Algorithm Performance Comparison =====" << endl;
    cout << "┌─────────────────┬────────────┬────────────┬────────────┬───────────┬────────────┐" << endl;
    cout << "│ Algorithm       │ Total Time │ Data Fetch │ Algorithm  │ Nodes     │ Path       │" << endl;
    cout << "│                 │ (ms)       │ Time (ms)  │ Time (ms)  │ Visited   │ Length     │" << endl;
    cout << "├─────────────────┼────────────┼────────────┼────────────┼───────────┼────────────┤" << endl;

    cout << "│ Bidirectional   │ " << setw(10) << fixed << setprecision(2) << biResult.time_ms
         << " │ " << setw(10) << biResult.data_fetch_ms
         << " │ " << setw(10) << biResult.algorithm_ms
         << " │ " << setw(9) << biResult.visited
         << " │ " << setw(10) << (biResult.path.empty() ? 0 : biResult.path.size()) << " │" << endl;

    cout << "│ BFS             │ " << setw(10) << fixed << setprecision(2) << bfsResult.time_ms
         << " │ " << setw(10) << bfsResult.data_fetch_ms
         << " │ " << setw(10) << bfsResult.algorithm_ms
         << " │ " << setw(9) << bfsResult.visited
         << " │ " << setw(10) << (bfsResult.path.empty() ? 0 : bfsResult.path.size()) << " │" << endl;

    cout << "└─────────────────┴────────────┴────────────┴────────────┴───────────┴────────────┘" << endl;

    // Clean up
    delete graph;
    if (outputFile.is_open()) {
        outputFile.close();
    }

    return 0;
}