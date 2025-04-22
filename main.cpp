#include <iostream>
#include "graph.h"

//function to display the path
void displayPath(const SearchResult& result, const string& algorithm, ostream& out = cout) {
    if (result.path.empty()) {
        out << "No path found using " << algorithm << "." << endl;
        return;
    }

    out << "\n======= " << algorithm << " Path =======" << endl;
    out << "Algorithm time: " << fixed << setprecision(2) << result.algorithm_ms << " ms" << endl;
    out << "Nodes visited: " << result.visited << endl;

    // prints path with movie connection
    out << "\nPath: ";
    for (size_t i = 0; i < result.path.size(); i++) {
        const auto& step = result.path[i];
        out << step.actor->name;
        if (i < result.path.size() - 1) {
            const auto& nextStep = result.path[i + 1];

            if (nextStep.movie && nextStep.prevActor && nextStep.prevActor->id == step.actor->id) {
                string year = nextStep.movie->release_date.empty() ? "N/A" : nextStep.movie->release_date.substr(0, 4);
                out << " → [" << nextStep.movie->title << " (" << year << ")] → ";
            } else {
                out << " → [Connection exists but movie details not available] → ";
            }
        }
    }
    out << endl;
}

//TODO: function to compare searches
void displayPerformances(const SearchResult& bds, const SearchResult& bfs, ostream& out = cout) {
    out << "\n===== Algorithm Performance Comparison =====" << endl;

    // Create a table header
    out << "┌─────────────────┬────────────┬────────────┐" << endl;
    out << "│ Algorithm       │ Algorithm  │ Nodes      │" << endl;
    out << "│                 │ Time (ms)  │ Visited    │" << endl;
    out << "├─────────────────┼────────────┼────────────┤" << endl;

    // Add BDS data
    out << "│ Bidirectional   │ " << setw(10) << fixed << setprecision(2) << bds.algorithm_ms
         << " │ " << setw(10) << bds.visited << " │" << endl;

    // Add BFS data
    out << "│ BFS             │ " << setw(10) << fixed << setprecision(2) << bfs.algorithm_ms
         << " │ " << setw(10) << bfs.visited << " │" << endl;

    out << "└─────────────────┴────────────┴────────────┘" << endl;


    // Add efficiency comparison
    out << "\n===== Efficiency Analysis =====" << endl;

    // Compare algorithm time
    if (bds.algorithm_ms < bfs.algorithm_ms) {
        double speedup = bfs.algorithm_ms / bds.algorithm_ms;
        out << "Bidirectional Search was " << fixed << setprecision(2) << speedup
            << "x faster than BFS." << endl;
    } else if (bfs.algorithm_ms < bds.algorithm_ms) {
        double speedup = bds.algorithm_ms / bfs.algorithm_ms;
        out << "BFS was " << fixed << setprecision(2) << speedup
            << "x faster than Bidirectional Search." << endl;
    } else {
        out << "Both algorithms had similar execution times." << endl;
    }

    // Compare nodes visited
    if (bds.visited < bfs.visited) {
        double reduction = static_cast<double>(bfs.visited) / bds.visited;
        out << "Bidirectional Search visited " << fixed << setprecision(2) << reduction
            << "x fewer nodes than BFS." << endl;
    } else if (bfs.visited < bds.visited) {
        double reduction = static_cast<double>(bds.visited) / bfs.visited;
        out << "BFS visited " << fixed << setprecision(2) << reduction
            << "x fewer nodes than Bidirectional Search." << endl;
    } else {
        out << "Both algorithms visited a similar number of nodes." << endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(){

    cout << "============== Welcome to StarPath! ==============\n" << endl;

    //initializing objects
    api tmdb("07663db07b6982f498aef71b6b0997f7");
    string actorName1, actorName2;

    //getting user input
    cout << "Enter name of the first actor: " << endl;
    getline(cin, actorName1);
    int actorId1 = tmdb.searchActor(actorName1);
    auto actor1 = tmdb.getActor(actorId1);
    if(actor1 == NULL || actor1->name != actorName1) {
        cerr << "No actors named '" << actorName1 << "' were found. Exiting program..." << endl;
        return 1;
    }

    cout << "Enter name of the second actor: " << endl;
    getline(cin, actorName2);

    int actorId2 = tmdb.searchActor(actorName2);
    auto actor2 = tmdb.getActor(actorId2);
    if(actor2 == NULL || actor2->name != actorName2) {
        cerr << "No actors named '" << actorName2 << "' were found. Exiting program..." << endl;
        return 1;
    }

    //build graph
    cout << "--------------------------------------------------" << endl;

    cout << "Creating constelation...\n" << endl;

    Graph graph(tmdb);
    graph.addActor(actor1, actorId2);
    graph.addActor(actor2, actorId1);

    auto stats = graph.getStats();
    cout << "Graph build with " << stats.first << " actors and " << stats.second << " connections." << endl;

    //perform searches
    cout << "--------------------------------------------------" << endl;
    cout << "Connecting stars...\n" << endl;

    cout << "Performing bidirectional search..." << endl;
    auto bidirectional = graph.findPathBDS(actorId1, actorId2);

    cout << "Performing breadth first search..." << endl;
    auto breathfirst = graph.findPathBFS(actorId1, actorId2);

    //write results to file
    cout << "--------------------------------------------------" << endl;
    cout << "Saving to file...\n" << endl;

    ofstream outputFile("results.txt");
    displayPath(bidirectional, "Bidirectional Search", outputFile);
    displayPath(breathfirst, "Breadth First Search", outputFile);

    //compare algorithms
    cout << "--------------------------------------------------" << endl;
    cout << "Comparing searches...\n" << endl;

    displayPerformances(bidirectional, breathfirst, outputFile);

    //close file
    if(outputFile.is_open()) {
        outputFile.close();
    }

    //clean up
    delete actor1;
    delete actor2;

    return 0;
}