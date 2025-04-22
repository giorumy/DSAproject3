#include "graph.h"
#include "unordered_set"

void Graph::addNode(Actor* actor) {
    cout << actor->name << " was added to graph." << endl;
    actors[actor->id] = actor;
    adjacencyList[actor->id] = vector<Connection>();
}

void Graph::findSharedMovie(Actor *actor, int targetActorID) {
    cout << "Fetching movies for " << actor->name << " to find connection to actor ID " << targetActorID << endl;
    vector<Movie> moviesArray = API.getMovies(actor->name);

    if(moviesArray.empty()) {
        cerr << "No movies found for " <<actor->name << endl;
        return;
    }

    bool found = false;
    vector<Actor> cast;

    for(auto& movie : moviesArray) {
        cout << "Checking movie: " << movie.title << " for connections" << endl;
        cast = API.getActors(movie.title);

        if(cast.empty()) {
            cerr << "No cast found for movie: " << movie.title << endl;
            continue;
        }

        for(auto& person : cast) {
            if(person.id == targetActorID) {
                cout << "Found connection: " << actor->name << " and actor ID " << person.id
                             << " in movie \"" << movie.title << "\"" << endl;

                Movie* moviePtr = addMovie(movie.id, movie.title, movie.release_date, movie.poster_path);

                //add target actor if not already in graph
                if(actors.find(person.id) == actors.end()) {
                    Actor* targetActor = new Actor(person.id, person.name, person.profile_path);
                    addNode(targetActor);
                }

                //add connection
                addConnection(actor->id, person.id, moviePtr);
                found = true;
                break; //found a connection -> no need to check other actors in this movie
            }
        }
        if(found) break; //found a connection -> no need to check other movies
    }
}


void Graph::addActor(Actor* actor, int targetActorID) {

    //in case of duplicates, deletes actor and returns
    if(actors.find(actor->id) != actors.end()) {
        delete actor;
        return;
    }

    addNode(actor);

    if(targetActorID > 0) {
        findSharedMovie(actor, targetActorID);
    }

}

Movie* Graph::addMovie(int id, const string& title, const string& release_date, const string& poster_path) {
    //check if movie already exists
    auto it = movies.find(id);
    if(it != movies.end()) {
        return it->second;
    }

    //else create and add new movie
    Movie* movie = new Movie(id, title, release_date, poster_path);
    movies[id] = movie;
    return movie;
}

//add a connection between actors through a movie
bool Graph::addConnection(int actorId1, int actorId2, Movie* movie) {
    if (adjacencyList.find(actorId1) == adjacencyList.end() ||
        adjacencyList.find(actorId2) == adjacencyList.end()) {
        return false;
    }

    //add connection from actorId1 to actorId2
    auto& connections1 = adjacencyList[actorId1];
    bool found1 = false;

    for (auto& conn : connections1) {
        if (conn.actorId == actorId2) {
            bool movieExists = false;
            for (const auto& m : conn.movies) {
                if (m->id == movie->id) {
                    movieExists = true;
                    break;
                }
            }

            if (!movieExists) {
                conn.movies.push_back(movie);
            }

            found1 = true;
            break;
        }
    }

    //connection not found -> create new one
    if (!found1) {
        Connection newConn(actorId2);
        newConn.movies.push_back(movie);
        connections1.push_back(newConn);
    }

    //add connection from actorId1 to actorId2
    auto& connections2 = adjacencyList[actorId2];
    bool found2 = false;

    for (auto& conn : connections2) {
        if (conn.actorId == actorId1) {
            bool movieExists = false;
            for (const auto& m : conn.movies) {
                if (m->id == movie->id) {
                    movieExists = true;
                    break;
                }
            }

            if (!movieExists) {
                conn.movies.push_back(movie);
            }

            found2 = true;
            break;
        }
    }

    //connection not found -> create new one
    if (!found2) {
        Connection newConn(actorId1);
        newConn.movies.push_back(movie);
        connections2.push_back(newConn);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Helper method for bidirectional search to process neighbors and find meeting points
bool Graph::processNeighbors(int currentActorId, unordered_map<int, pair<int, Movie*>>& previous, set<int> &visited, queue<int> &q, set<int> &otherVisited, int &meetingPoint) {
    const auto& connections = adjacencyList[currentActorId];
    //For every actor in the adjacency list for the current actor:
    for(const auto& connection : connections) {
        int neighboringActorId = connection.actorId;
        //Check if they are in the visited set, if not then add them and check to see if we find a meeting point:
        if(visited.find(neighboringActorId) == visited.end()) {
            visited.insert(neighboringActorId);
            q.push(neighboringActorId);
            previous[neighboringActorId] = make_pair(currentActorId, connection.movies[0]);
            //If they are in otherVisited then we found a meeting point:
            if(otherVisited.find(neighboringActorId) != otherVisited.end()) {
                meetingPoint = neighboringActorId;
                return true;
            }
        }
    }
    //If we do not find a meeting point, return false
    return false;
}

// BFS to find path between two actors
SearchResult Graph::findPathBFS(int startActorId, int endActorId) {

    auto start_time = chrono::high_resolution_clock::now(); //start timer

    SearchResult result;

    cout << "----------------------------------------------------------------------------------------------------------\n" << endl;
    cout << "BFS from actor " << startActorId << " to " << endActorId << ":\n" <<endl;

    if(startActorId == endActorId) { //trying to connect an actor to themselves
        if (actors.find(startActorId) != actors.end()) {
            result.path.push_back(PathStep(actors[startActorId]));
            result.visited = 1;
        }
        auto end_time = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(end_time - start_time).count();
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //step 1: fetch data in case start actor has no edges yet

    auto fetch_start = chrono::high_resolution_clock::now();

    if (adjacencyList[startActorId].empty()) {
        cout << "BFS: No existing connections for actor " << startActorId << ". ";
        set<int> dummySet;
        expandFromActor(startActorId, dummySet);
    } else {
        cout << "BFS: Found " << adjacencyList[startActorId].size() << " existing connections for start actor." << endl;
    }

    auto fetch_end = chrono::high_resolution_clock::now();


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //step 2: algorithm

    auto algorithm_start = chrono::high_resolution_clock::now();

    unordered_map<int, pair<int, Movie*>> previous; //to keep track of how we got to each actor -> stores (parent.id, edge)
    unordered_set<int> visited;
    queue<int> q;
    q.push(startActorId);
    visited.insert(startActorId);
    bool found = false;
    result.visited = 1;

    while (!q.empty() && !found) {
        int currentId = q.front();
        q.pop();

        cout << "BFS: Visiting actor " << currentId << " (" << actors[currentId]->name << ")" << endl;

        const auto& connections = adjacencyList[currentId];

        //explore neighbors
        for(const auto& connection : connections) {
            int neighborId = connection.actorId;

            if(visited.find(neighborId) == visited.end()) { //neighbor hasn't been visited yet
                cout << "BFS: Visiting neighbor " << neighborId  << " (" << actors[neighborId]->name << ")" << endl;

                visited.insert(neighborId);
                result.visited++;
                q.push(neighborId);

                previous[neighborId] = make_pair(currentId, connection.movies[0]);

                if(neighborId == endActorId) {
                    cout << "BFS: Found target actor! Search complete." << endl;
                    found = true;
                    break;
                }
            }
        }
    }

    auto end_time = chrono::high_resolution_clock::now(); //end timer
    result.time_ms = chrono::duration<double, milli>(end_time - start_time).count();


    if (!found) {
        cout << "BFS: No path found between actors." << endl;
        return result;
    }

    //reconstruct path to add to SearchResult
    vector<PathStep> reversedPath;
    int current = endActorId;

    while(current != startActorId) {
        auto [prev, movie] = previous[current];
        reversedPath.push_back(PathStep(actors[current], actors[prev], movie));
        current = prev;
    }
    reversedPath.push_back(PathStep(actors[startActorId])); //add start actor to path

    //reverse and add to result
    for (auto it = reversedPath.rbegin(); it != reversedPath.rend(); ++it) {
        result.path.push_back(*it);
    }

    cout << "BFS complete! Path length: " << result.path.size() << endl;
    return result;
}

//Expand from actor by checking movies to see if there are connecting actors.
void Graph::expandFromActor(int actorId, set<int>& targetSet) {

    if (actors.find(actorId) == actors.end()) {
        return; //actor is not on graph
    }

    Actor* actor = actors[actorId];

    cout << "Expanding graph from actor: " << actor->name << endl;

    //fetch movies for that actor
    vector<Movie> movies = API.getMovies(actor->name);
    if (movies.empty()) {
        cerr << "No movies found for " << actor->name << " during graph expansion" << endl;
        return;
    }

    //fetch cast for each movie actor is in
    for (auto& movie : movies) {
        vector<Actor> cast = API.getActors(movie.title);

        if (cast.empty()) {
            continue; //no cast data to work with
        }

        //go through each person in the cast
        for (auto& person : cast) {
            if (person.id == actorId) { //self-connections
                continue;
            }

            //add new actor, if not in the graph already
            if (actors.find(person.id) == actors.end()) {
                Actor* newActor = new Actor(person.id, person.name, person.profile_path);
                addNode(newActor);

                //add to target set for bidirectional search
                targetSet.insert(person.id);
            }

        }
    }
}

//Bidirectional Search (BDS)
SearchResult Graph::findPathBDS(int startActorId, int endActorId) {
    auto startTime = chrono::high_resolution_clock::now();
    SearchResult result;

    if(startActorId == endActorId) {
        if(actors.find(startActorId) != actors.end()) {
            result.path.push_back(PathStep(actors[startActorId]));
            result.visited = 1;
        }
        auto endTime = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
        return result;
    }
    //Expand graph from starting actor if empty
    if(adjacencyList[startActorId].empty()) {
        set<int> expansionSet;
        expandFromActor(startActorId, expansionSet);
    }
    //Expand graph from ending actor if empty
    if(adjacencyList[endActorId].empty()) {
        set<int> expansionSet;
        expandFromActor(endActorId, expansionSet);
    }
    int meetingPoint = -1;
    result.visited = 2;
    //Forward search starting with starting actor
    queue<int> forwardQueue;
    set<int> forwardVisited;
    unordered_map<int, pair<int, Movie*>> forwardPrevious;
    forwardQueue.push(startActorId);
    //Backwards search starting with ending actor
    queue<int> backwardQueue;
    set<int> backwardVisited;
    unordered_map<int, pair<int, Movie*>> backwardPrevious;
    backwardQueue.push(endActorId);
    //Breadth First Search in both directions
    while(!forwardQueue.empty() && !backwardQueue.empty() && meetingPoint == -1) {
        int levelSize = forwardQueue.size();
        for(int i = 0; i < levelSize && meetingPoint == -1; i++) {
            int currentActorId = forwardQueue.front();
            forwardQueue.pop();
            if(processNeighbors(currentActorId, forwardPrevious, forwardVisited, forwardQueue, backwardVisited, meetingPoint)) {
                break;
            }
            result.visited++;
        }
        if(meetingPoint != -1) {
            break;
        }
        levelSize = backwardQueue.size();
        for (int i = 0; i < levelSize && meetingPoint == -1; i++) {
            int currentId = backwardQueue.front();
            backwardQueue.pop();

            if (processNeighbors(currentId, backwardPrevious, backwardVisited, backwardQueue, forwardVisited, meetingPoint)) {
                break;
            }
            result.visited++;
        }
    }
    //Reconstructing path if a meeting point is found
    if(meetingPoint != -1) {
        vector<PathStep> forwardPath;
        vector<PathStep> backwardPath;
        //Building path forward from starting actor
        int currentActorId = meetingPoint;
        while(currentActorId != startActorId) {
            auto prev = forwardPrevious[currentActorId];
            forwardPath.insert(forwardPath.begin(), PathStep(actors[currentActorId], actors[prev.first], prev.second));
            currentActorId = prev.first;
        }
        forwardPath.insert(forwardPath.begin(), PathStep(actors[startActorId]));
        //Building path backwards from ending actor
        currentActorId = meetingPoint;
        while(backwardPrevious.find(currentActorId) != backwardPrevious.end()) {
            auto prev = backwardPrevious[currentActorId];
            backwardPath.push_back(PathStep(actors[prev.first], actors[currentActorId], prev.second));
            currentActorId = prev.first;
        }
        // Combine paths (remove duplicate meeting point)
        result.path = forwardPath;
        if (!backwardPath.empty()) {
            result.path.insert(result.path.end(), backwardPath.begin(), backwardPath.end());
        }
    }
    auto endTime = chrono::high_resolution_clock::now();
    result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
    return result;
}

pair<int, int> Graph::getStats() const {
    int connectionCount = 0;

    for (const auto& pair : adjacencyList) {
        connectionCount += pair.second.size();
    }

    //divide by 2 because it's bidirectional
    return make_pair(actors.size(), connectionCount / 2);
}

