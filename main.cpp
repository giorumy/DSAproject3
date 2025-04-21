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
    out << "Data fetch time: " << fixed << setprecision(2) << " ms" << endl;
    out << "Algorithm time: " << fixed << setprecision(2) << " ms" << endl;
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
    api api("07663db07b6982f498aef71b6b0997f7");
    Graph graph;
    auto actor = api.getActor(31);
    auto actor2 = api.getActor(32);
    auto result1 = graph.findPathBFS(actor->id, actor2->id);
    auto result = graph.findPathBDS(actor->id, actor2->id);
    displayPath(result1, "bfs", cout);
    displayPath(result, "bds", cout);
    cout << actor->name  << " " << actor2->name << endl;

    return 0;
}
