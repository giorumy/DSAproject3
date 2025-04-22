#include <iostream>
#include "graph.h"


//TODO: Function to display the path
void displayPath(const SearchResult& result, const string& algorithm, ostream& out = cout) {
    if (result.path.empty()) {
        out << "No path found using " << algorithm << "." << endl;
        return;
    }

    out << "\n===== Path found using " << algorithm << " =====" << endl;
    out << "Total time: " << fixed << setprecision(2) << result.time_ms << " ms" << endl;
    out << "Data fetch time: " << fixed << setprecision(2) << result.time_ms << " ms" << endl;
    out << "Algorithm time: " << fixed << setprecision(2) << result.time_ms << " ms" << endl;
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

int main()
{
    std::cout << "Welcome to StarPath!" << std::endl;

    api tmdb("07663db07b6982f498aef71b6b0997f7");
    Graph graph;


    int margotId = tmdb.searchActor("Willem Dafoe");
    int bradId = tmdb.searchActor("Carey Mulligan");

    Actor* margot = tmdb.getActor(margotId);
    Actor* brad = tmdb.getActor(bradId);

    if (!margot || !brad) {
        cout << "Failed to get actor details." << endl;
        return 1;
    }

    cout << "Adding " << margot->name << " to graph..." << endl;
    graph.addActor(margot, bradId);

    cout << "Adding " << brad->name << " to graph..." << endl;
    graph.addActor(brad, margotId);

    auto stats = graph.getStats();
    cout << "\nGraph built with " << stats.first << " actors and " << stats.second << " connections." << endl;

    cout << "\nFinding path using BDS from " << margot->name << " to " << brad->name << "..." << endl;
    SearchResult bds = graph.findPathBDS(margotId, bradId);

    //output result

    // Open output file
    ofstream outputFile("starpath_results.txt");
    if (outputFile.is_open()) {
        outputFile << "=== StarPath Results: " << margot << " to " << brad << " ===" << endl;
        displayPath(bds, "BdS", outputFile);
    } else {
        cerr << "Failed to open output file for writing results." << endl;
    }



    //TODO:
    //getting user input
    //api client
    //get actors from api
    //add actors to graph
    //traverse using algorithms
    //display paths
    //performance analysis

    return 0;
}