#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include "graph.h"
#include "api.h"  // Updated include

using namespace std;

// Function to display the path
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
    out << "\nPath:" << endl;

    for (size_t i = 0; i < result.path.size(); i++) {
        const auto& step = result.path[i];
        out << i + 1 << ". " << step.actor->name << endl;

        if (i < result.path.size() - 1 && step.movie) {
            string year = "N/A";
            if (!step.movie->release_date.empty()) {
                year = step.movie->release_date.substr(0, 4);
            }
            out << "   ↓ appeared in \"" << step.movie->title << "\" (" << year << ") with" << endl;
        }
    }
}

int main() {
    cout << "=== StarPath: Find connections between actors ===\n" << endl;

    // Open output file
    ofstream outputFile("starpath_results.txt");
    if (!outputFile.is_open()) {
        cerr << "Failed to open output file for writing results." << endl;
        // Continue with console output only
    }

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

    // Calculate performance differences
    double timeDiff = bfsResult.time_ms - biResult.time_ms;
    double algoTimeDiff = bfsResult.algorithm_ms - biResult.algorithm_ms;
    int visitedDiff = bfsResult.visited - biResult.visited;

    cout << "\n===== Performance Analysis =====" << endl;

    // Time comparison
    cout << "Time difference: ";
    if (timeDiff > 0) {
        cout << "Bidirectional is " << fixed << setprecision(2) << (timeDiff / bfsResult.time_ms * 100)
             << "% faster overall" << endl;
    } else if (timeDiff < 0) {
        cout << "BFS is " << fixed << setprecision(2) << (-timeDiff / biResult.time_ms * 100)
             << "% faster overall" << endl;
    } else {
        cout << "Both algorithms have the same overall time" << endl;
    }

    // Algorithm time comparison
    cout << "Algorithm time difference: ";
    if (algoTimeDiff > 0) {
        cout << "Bidirectional is " << fixed << setprecision(2) << (algoTimeDiff / bfsResult.algorithm_ms * 100)
             << "% faster in algorithm execution" << endl;
    } else if (algoTimeDiff < 0) {
        cout << "BFS is " << fixed << setprecision(2) << (-algoTimeDiff / biResult.algorithm_ms * 100)
             << "% faster in algorithm execution" << endl;
    } else {
        cout << "Both algorithms have the same algorithm execution time" << endl;
    }

    // Nodes visited comparison
    cout << "Nodes visited difference: ";
    if (visitedDiff > 0) {
        cout << "Bidirectional visited " << visitedDiff << " fewer nodes ("
             << fixed << setprecision(2) << (static_cast<double>(visitedDiff) / bfsResult.visited * 100)
             << "% reduction)" << endl;
    } else if (visitedDiff < 0) {
        cout << "BFS visited " << -visitedDiff << " fewer nodes ("
             << fixed << setprecision(2) << (static_cast<double>(-visitedDiff) / biResult.visited * 100)
             << "% reduction)" << endl;
    } else {
        cout << "Both algorithms visited the same number of nodes" << endl;
    }

    // Path length comparison
    if (!biResult.path.empty() && !bfsResult.path.empty()) {
        int biLength = biResult.path.size();
        int bfsLength = bfsResult.path.size();

        cout << "Path length: ";
        if (biLength < bfsLength) {
            cout << "Bidirectional found a shorter path" << endl;
        } else if (bfsLength < biLength) {
            cout << "BFS found a shorter path" << endl;
        } else {
            cout << "Both algorithms found paths of equal length" << endl;
        }
    }

    // Clean up
    delete graph;
    if (outputFile.is_open()) {
        outputFile.close();
    }

    return 0;
}
