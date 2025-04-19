#include "graph.h"

// Add an actor to the graph, fetching only movies connecting to the target actor
void Graph::addActor(Actor* actor, int targetActorId) {
    if (actors.find(actor->id) == actors.end()) {
        actors[actor->id] = actor;
        adjacencyList[actor->id] = vector<Connection>(); // Initialize empty connections

        if (targetActorId > 0) {
            cout << "Fetching movies for " << actor->name << " to find connection to actor ID " << targetActorId << endl;
            vector<Movie> movies = data.getMovies(actor->name);

            if (movies.empty()) {
                cout << "Warning: No movies found for " << actor->name << endl;
                return; // Don't proceed if no movies found
            }

            bool foundConnection = false;
            vector<Actor> cast;

            for(auto& movie : movies) {
                cout << "Checking movie: " << movie.title << " for connections" << endl;
                cast = data.getActors(movie.title);

                if (cast.empty()) {
                    cout << "No cast found for movie: " << movie.title << endl;
                    continue;
                }

                for(auto& person : cast) {
                    if (person.id == targetActorId) { // Only add connection if it leads to the target
                        cout << "Found connection: " << actor->name << " and actor ID " << person.id
                             << " in movie \"" << movie.title << "\"" << endl;

                        // Create movie object
                        Movie* moviePtr = addMovie(movie.id, movie.title, movie.release_date, movie.poster_path);

                        // Add the target actor if not already in the graph
                        if (actors.find(person.id) == actors.end()) {
                            Actor* targetActor = new Actor(person.id, person.name, person.profile_path);
                            actors[person.id] = targetActor;
                            adjacencyList[person.id] = vector<Connection>();
                        }

                        // Add connection
                        addConnection(actor->id, person.id, moviePtr);
                        foundConnection = true;
                        break; // Found a connection, no need to check other actors in this movie
                    }
                }

                if (foundConnection) {
                    break; // Found a connection, no need to check other movies
                }
            }

            if (!foundConnection) {
                cout << "No direct connection found between " << actor->name
                     << " and actor with ID " << targetActorId << endl;
            }
        }
    } else {
        // Actor already exists, delete the new one to avoid memory leak
        delete actor;
    }
}

// Expand the graph from an actor (used in bidirectional search)
void Graph::expandFromActor(int actorId, set<int>& targetSet) {
    if (actors.find(actorId) == actors.end()) {
        return;
    }

    Actor* actor = actors[actorId];
    cout << "Expanding graph from actor: " << actor->name << endl;

    vector<Movie> movies = data.getMovies(actor->name);
    if (movies.empty()) {
        cout << "No movies found for " << actor->name << " during graph expansion" << endl;
        return;
    }

    int connectionsAdded = 0;

    for (auto& movie : movies) {
        vector<Actor> cast = data.getActors(movie.title);

        if (cast.empty()) {
            continue;
        }

        // Create movie object
        Movie* moviePtr = addMovie(movie.id, movie.title, movie.release_date, movie.poster_path);

        for (auto& person : cast) {
            // Skip self-connections
            if (person.id == actorId) {
                continue;
            }

            // Add the actor if not already in the graph
            if (actors.find(person.id) == actors.end()) {
                Actor* newActor = new Actor(person.id, person.name, person.profile_path);
                actors[person.id] = newActor;
                adjacencyList[person.id] = vector<Connection>();

                // Add to target set for bidirectional search
                targetSet.insert(person.id);
            }

            // Add connection
            if (addConnection(actorId, person.id, moviePtr)) {
                connectionsAdded++;
            }
        }
    }

    cout << "Added " << connectionsAdded << " connections from " << actor->name << endl;
}

void Graph::addMovie(Movie* movie) {
    movies.push_back(movie);
}

// Add a movie to the graph
Movie* Graph::addMovie(int id, const string& title, const string& release_date, const string& poster_path) {
    // Check if movie with this ID already exists
    for (auto movie : movies) {
        if (movie->id == id) {
            return movie;
        }
    }

    // Create new movie and add to collection
    Movie* movie = new Movie(id, title, release_date, poster_path);
    movies.push_back(movie);
    return movie;
}

// Add a connection between actors through a movie
bool Graph::addConnection(int actorId1, int actorId2, Movie* movie) {
    if (actors.find(actorId1) == actors.end() ||
        actors.find(actorId2) == actors.end()) {
        return false;
    }

    // Check if connection already exists
    auto& connections1 = adjacencyList[actorId1];
    bool found1 = false;

    for (auto& conn : connections1) {
        if (conn.actorId == actorId2) {
            // Check if movie already exists in this connection
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

    if (!found1) {
        // Create new connection
        Connection newConn(actorId2);
        newConn.movies.push_back(movie);
        connections1.push_back(newConn);
    }

    // Add the reverse connection
    auto& connections2 = adjacencyList[actorId2];
    bool found2 = false;

    for (auto& conn : connections2) {
        if (conn.actorId == actorId1) {
            // Check if movie already exists in this connection
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

    if (!found2) {
        // Create new connection
        Connection newConn(actorId1);
        newConn.movies.push_back(movie);
        connections2.push_back(newConn);
    }

    return true;
}

// Get actor by ID
Actor* Graph::getActor(int actorId) const {
    auto it = actors.find(actorId);
    if (it != actors.end()) {
        return it->second;
    }
    return nullptr;
}

// Helper method for bidirectional search
bool Graph::processNeighbors(int currentId, unordered_map<int, pair<int, Movie*>>& previous,
                           set<int>& visited, queue<int>& q, set<int>& otherVisited,
                           int& meetingPoint) {

    const auto& connections = adjacencyList[currentId];

    for (const auto& connection : connections) {
        int neighborId = connection.actorId;

        if (visited.find(neighborId) == visited.end()) {
            visited.insert(neighborId);
            q.push(neighborId);

            // Use the first movie for simplicity
            previous[neighborId] = make_pair(currentId, connection.movies[0]);

            // Check if we've found a meeting point
            if (otherVisited.find(neighborId) != otherVisited.end()) {
                meetingPoint = neighborId;
                return true;
            }
        }
    }

    return false;
}

// Bidirectional BFS for more efficient path finding
SearchResult Graph::findPathBidirectional(int startActorId, int endActorId) {
    auto totalStartTime = chrono::high_resolution_clock::now();
    SearchResult result;

    if (startActorId == endActorId) {
        if (actors.find(startActorId) != actors.end()) {
            result.path.push_back(PathStep(actors[startActorId]));
            result.visited = 1;
        }
        auto totalEndTime = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(totalEndTime - totalStartTime).count();
        return result;
    }

    // Data fetching phase - expand initial actors if needed
    auto dataFetchStart = chrono::high_resolution_clock::now();

    if (adjacencyList[startActorId].empty()) {
        cout << "Expanding graph from start actor (ID: " << startActorId << ")" << endl;
        set<int> dummySet;
        expandFromActor(startActorId, dummySet);
    }

    if (adjacencyList[endActorId].empty()) {
        cout << "Expanding graph from end actor (ID: " << endActorId << ")" << endl;
        set<int> dummySet;
        expandFromActor(endActorId, dummySet);
    }

    auto dataFetchEnd = chrono::high_resolution_clock::now();
    result.data_fetch_ms = chrono::duration<double, milli>(dataFetchEnd - dataFetchStart).count();

    // Algorithm phase
    auto algoStartTime = chrono::high_resolution_clock::now();

    // Forward search from start actor
    queue<int> forwardQueue;
    set<int> forwardVisited;
    unordered_map<int, pair<int, Movie*>> forwardPrevious;

    // Backward search from end actor
    queue<int> backwardQueue;
    set<int> backwardVisited;
    unordered_map<int, pair<int, Movie*>> backwardPrevious;

    // Initialize
    forwardQueue.push(startActorId);
    forwardVisited.insert(startActorId);

    backwardQueue.push(endActorId);
    backwardVisited.insert(endActorId);

    int meetingPoint = -1;
    result.visited = 2; // Start with both actors

    // Bidirectional BFS
    while (!forwardQueue.empty() && !backwardQueue.empty() && meetingPoint == -1) {
        // Process one level from forward direction
        int levelSize = forwardQueue.size();
        for (int i = 0; i < levelSize && meetingPoint == -1; i++) {
            int currentId = forwardQueue.front();
            forwardQueue.pop();

            if (processNeighbors(currentId, forwardPrevious, forwardVisited, forwardQueue,
                               backwardVisited, meetingPoint)) {
                break;
            }

            result.visited++;
        }

        if (meetingPoint != -1) break;

        // Process one level from backward direction
        levelSize = backwardQueue.size();
        for (int i = 0; i < levelSize && meetingPoint == -1; i++) {
            int currentId = backwardQueue.front();
            backwardQueue.pop();

            if (processNeighbors(currentId, backwardPrevious, backwardVisited, backwardQueue,
                               forwardVisited, meetingPoint)) {
                break;
            }

            result.visited++;
        }
    }

    // Reconstruct path if a meeting point was found
    if (meetingPoint != -1) {
        // Build forward path
        vector<PathStep> forwardPath;
        int current = meetingPoint;

        while (current != startActorId) {
            auto prev = forwardPrevious[current];
            forwardPath.insert(forwardPath.begin(), PathStep(
                actors[current],
                actors[prev.first],
                prev.second
            ));
            current = prev.first;
        }

        forwardPath.insert(forwardPath.begin(), PathStep(actors[startActorId]));

        // Build backward path
        vector<PathStep> backwardPath;
        current = meetingPoint;

        while (backwardPrevious.find(current) != backwardPrevious.end()) {
            auto prev = backwardPrevious[current];
            backwardPath.push_back(PathStep(
                actors[prev.first],
                actors[current],
                prev.second
            ));
            current = prev.first;
        }

        // Combine paths (remove duplicate meeting point)
        result.path = forwardPath;
        if (!backwardPath.empty()) {
            result.path.insert(result.path.end(), backwardPath.begin(), backwardPath.end());
        }
    }

    auto algoEndTime = chrono::high_resolution_clock::now();
    result.algorithm_ms = chrono::duration<double, milli>(algoEndTime - algoStartTime).count();

    auto totalEndTime = chrono::high_resolution_clock::now();
    result.time_ms = chrono::duration<double, milli>(totalEndTime - totalStartTime).count();
    return result;
}

// BFS to find path between two actors
SearchResult Graph::findPathBFS(int startActorId, int endActorId) {
    auto totalStartTime = chrono::high_resolution_clock::now();
    SearchResult result;

    if (startActorId == endActorId) {
        if (actors.find(startActorId) != actors.end()) {
            result.path.push_back(PathStep(actors[startActorId]));
            result.visited = 1;
        }
        auto totalEndTime = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(totalEndTime - totalStartTime).count();
        return result;
    }

    // Data fetching phase - ensure we have data for the start actor
    auto dataFetchStart = chrono::high_resolution_clock::now();

    if (adjacencyList[startActorId].empty()) {
        cout << "Expanding graph from start actor for BFS (ID: " << startActorId << ")" << endl;
        set<int> dummySet;
        expandFromActor(startActorId, dummySet);
    }

    auto dataFetchEnd = chrono::high_resolution_clock::now();
    result.data_fetch_ms = chrono::duration<double, milli>(dataFetchEnd - dataFetchStart).count();

    // Algorithm phase
    auto algoStartTime = chrono::high_resolution_clock::now();

    queue<int> q;
    set<int> visited;
    unordered_map<int, pair<int, Movie*>> previous;

    q.push(startActorId);
    visited.insert(startActorId);
    result.visited = 1;

    while (!q.empty()) {
        int currentActorId = q.front();
        q.pop();

        // Get all connections for the current actor
        const auto& connections = adjacencyList[currentActorId];

        for (const auto& connection : connections) {
            int neighborId = connection.actorId;

            if (visited.find(neighborId) == visited.end()) {
                visited.insert(neighborId);
                result.visited++;
                q.push(neighborId);

                // Use the first movie for simplicity
                previous[neighborId] = make_pair(currentActorId, connection.movies[0]);

                if (neighborId == endActorId) {
                    // Reconstruct path
                    vector<PathStep> path;
                    int current = endActorId;

                    while (current != startActorId) {
                        auto prev = previous[current];
                        path.insert(path.begin(), PathStep(
                            actors[current],
                            actors[prev.first],
                            prev.second
                        ));
                        current = prev.first;
                    }

                    path.insert(path.begin(), PathStep(actors[startActorId]));
                    result.path = path;

                    auto algoEndTime = chrono::high_resolution_clock::now();
                    result.algorithm_ms = chrono::duration<double, milli>(algoEndTime - algoStartTime).count();

                    auto totalEndTime = chrono::high_resolution_clock::now();
                    result.time_ms = chrono::duration<double, milli>(totalEndTime - totalStartTime).count();
                    return result;
                }
            }
        }
    }

    auto algoEndTime = chrono::high_resolution_clock::now();
    result.algorithm_ms = chrono::duration<double, milli>(algoEndTime - algoStartTime).count();

    auto totalEndTime = chrono::high_resolution_clock::now();
    result.time_ms = chrono::duration<double, milli>(totalEndTime - totalStartTime).count();
    return result;
}

// DFS to find path between two actors
SearchResult Graph::findPathDFS(int startActorId, int endActorId) {
    auto totalStartTime = chrono::high_resolution_clock::now();
    SearchResult result;

    if (startActorId == endActorId) {
        if (actors.find(startActorId) != actors.end()) {
            result.path.push_back(PathStep(actors[startActorId]));
            result.visited = 1;
        }
        auto totalEndTime = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(totalEndTime - totalStartTime).count();
        return result;
    }

    // Data fetching phase - ensure we have data for the start actor
    auto dataFetchStart = chrono::high_resolution_clock::now();

    if (adjacencyList[startActorId].empty()) {
        set<int> dummySet;
        expandFromActor(startActorId, dummySet);
    }

    auto dataFetchEnd = chrono::high_resolution_clock::now();
    result.data_fetch_ms = chrono::duration<double, milli>(dataFetchEnd - dataFetchStart).count();

    // Algorithm phase
    auto algoStartTime = chrono::high_resolution_clock::now();

    set<int> visited;
    vector<PathStep> path;

    function<bool(int, int, vector<PathStep>&)> dfsHelper = [&](int currentId, int targetId, vector<PathStep>& currentPath) -> bool {
        visited.insert(currentId);
        result.visited++;

        if (currentId == targetId) {
            return true;
        }

        const auto& connections = adjacencyList[currentId];

        for (const auto& connection : connections) {
            int neighborId = connection.actorId;

            if (visited.find(neighborId) == visited.end()) {
                currentPath.push_back(PathStep(
                    actors[neighborId],
                    actors[currentId],
                    connection.movies[0] // Just use the first movie for simplicity
                ));

                if (dfsHelper(neighborId, targetId, currentPath)) {
                    return true;
                }

                currentPath.pop_back();
            }
        }

        return false;
    };

    path.push_back(PathStep(actors[startActorId]));
    bool found = dfsHelper(startActorId, endActorId, path);

    if (found) {
        result.path = path;
    }

    auto algoEndTime = chrono::high_resolution_clock::now();
    result.algorithm_ms = chrono::duration<double, milli>(algoEndTime - algoStartTime).count();

    auto totalEndTime = chrono::high_resolution_clock::now();
    result.time_ms = chrono::duration<double, milli>(totalEndTime - totalStartTime).count();
    return result;
}

// Get graph statistics
pair<int, int> Graph::getStats() const {
    int connectionCount = 0;
    for (const auto& pair : adjacencyList) {
        connectionCount += pair.second.size();
    }

    // Divide by 2 because connections are bidirectional
    return make_pair(actors.size(), connectionCount / 2);
}