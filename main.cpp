#include <iostream>
#include "graph.h"

//TODO: Function to display the path
void displayPath(const SearchResult& result, const string& algorithm, ostream& out = cout) {}

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

    cout << "--------------------------------------------------" << endl;

    cout << "Creating constelation...\n" << endl;

    //build graph
    Graph graph(tmdb);

    graph.addActor(actor1, actorId2);
    graph.addActor(actor2, actorId1);

    auto stats = graph.getStats();
    cout << "Graph build with " << stats.first << " actors and " << stats.second << " connections." << endl;

    //perform searches
    cout << "--------------------------------------------------" << endl;
    cout << "Connecting stars...\n" << endl;

    //write results to file
    cout << "--------------------------------------------------" << endl;
    cout << "Saving to file...\n" << endl;
    ofstream outputFile("results.txt");

    //compare algorithms
    cout << "--------------------------------------------------" << endl;
    cout << "Saving to file...\n" << endl;

    //close file
    if(outputFile.is_open()) {
        outputFile.close();
    }

    return 0;
}

//TODO:
//getting user input
//api client
//get actors from api
//add actors to graph
//traverse using algorithms
//display paths
//performance analysis
