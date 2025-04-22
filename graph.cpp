//
// Created by gioru on 4/10/2025.
//

//basic structure

#include "graph.h"

// Add an actor to the graph
void Graph::addActor(Actor* actor, Actor* actorTwo) {
    if (actors.find(actor->id) == actors.end()) {
        actors[actor->id] = actor;
        cout << "Getting movies for actor: " << actor->name << endl;
        vector<Movie> movies = data.getMovies(actor->name);
        vector<Actor> cast;
        for(auto movie : movies) {
            cout << movie.title << ": ";
            cast = data.getActors(movie.title);
            for(auto person : cast) {
                if (person.id == actorTwo->id) {
                    cout << " \n Match found! \n";
                    cout << "Path: " << actor->name << " " << movie.title << " " << actorTwo->name;
                    goto exitCondition;
                }
                cout << person.name << ", ";
                addConnection(actor->id, person.id, &movie);
            }
            cout << endl;
        }
    } else {
        // Actor already exists, delete the new one to avoid memory leak
        delete actor;
    }
    exitCondition:
    ;
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
    if (adjacencyList.find(actorId1) == adjacencyList.end() ||
        adjacencyList.find(actorId2) == adjacencyList.end()) {
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

// BFS to find path between two actors
SearchResult Graph::findPathBFS(int startActorId, int endActorId) {
    auto startTime = chrono::high_resolution_clock::now();
    SearchResult result;

    if (startActorId == endActorId) {
        if (actors.find(startActorId) != actors.end()) {
            result.path.push_back(PathStep(actors[startActorId]));
            result.visited = 1;
        }
        auto endTime = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
        return result;
    }

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
        //TODO:
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

                    auto endTime = chrono::high_resolution_clock::now();
                    result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
                    return result;
                }
            }
        }
    }

    auto endTime = chrono::high_resolution_clock::now();
    result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
    return result;
}

// DFS to find path between two actors
SearchResult Graph::findPathDFS(int startActorId, int endActorId) {
    auto startTime = chrono::high_resolution_clock::now();
    SearchResult result;

    if (startActorId == endActorId) {
        if (actors.find(startActorId) != actors.end()) {
            result.path.push_back(PathStep(actors[startActorId]));
            result.visited = 1;
        }
        auto endTime = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
        return result;
    }

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
                    connection.movies[0]
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

    auto endTime = chrono::high_resolution_clock::now();
    result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
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


// Bidirectional search to find path between two actors
SearchResult Graph::findPathBDS(int startActorId, int endActorId) {
    auto startTime = chrono::high_resolution_clock::now();
    SearchResult result;
    if (startActorId == endActorId) {
        if (actors.find(startActorId) != actors.end()) {
            result.path.push_back(PathStep(actors[startActorId]));
            result.visited = 1;
        }
        auto endTime = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
        return result;
    }


    if (actors.find(startActorId) == actors.end() || actors.find(endActorId) == actors.end()) {
        auto endTime = chrono::high_resolution_clock::now();
        result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
        return result;
    }

    queue<int> forwardQueue;
    set<int> forwardVisited;
    unordered_map<int, pair<int, Movie*>> forwardPrevious;
    queue<int> backwardQueue;
    set<int> backwardVisited;
    unordered_map<int, pair<int, Movie*>> backwardPrevious;
    forwardQueue.push(startActorId);
    forwardVisited.insert(startActorId);
    backwardQueue.push(endActorId);
    backwardVisited.insert(endActorId);
    int meetingPoint = -1;
    result.visited = 2;

    while (!forwardQueue.empty() && !backwardQueue.empty() && meetingPoint == -1) {
        int forwardSize = forwardQueue.size();
        for (int i = 0; i < forwardSize && meetingPoint == -1; i++) {
            int currentActorId = forwardQueue.front();
            forwardQueue.pop();
            const auto& connections = adjacencyList[currentActorId];

            for (const auto& connection : connections) {
                int neighborId = connection.actorId;
                if (backwardVisited.find(neighborId) != backwardVisited.end()) {
                    meetingPoint = neighborId;
                    forwardPrevious[neighborId] = make_pair(currentActorId, connection.movies[0]);
                    break;
                }
                if (forwardVisited.find(neighborId) == forwardVisited.end()) {
                    forwardVisited.insert(neighborId);
                    result.visited++;
                    forwardQueue.push(neighborId);
                    forwardPrevious[neighborId] = make_pair(currentActorId, connection.movies[0]);
                }
            }
        }
        if (meetingPoint != -1) break;
        int backwardSize = backwardQueue.size();
        for (int i = 0; i < backwardSize && meetingPoint == -1; i++) {
            int currentActorId = backwardQueue.front();
            backwardQueue.pop();
            const auto& connections = adjacencyList[currentActorId];

            for (const auto& connection : connections) {
                int neighborId = connection.actorId;
                if (forwardVisited.find(neighborId) != forwardVisited.end()) {
                    meetingPoint = neighborId;
                    backwardPrevious[neighborId] = make_pair(currentActorId, connection.movies[0]);
                    break;
                }
                if (backwardVisited.find(neighborId) == backwardVisited.end()) {
                    backwardVisited.insert(neighborId);
                    result.visited++;
                    backwardQueue.push(neighborId);
                    backwardPrevious[neighborId] = make_pair(currentActorId, connection.movies[0]);
                }
            }
        }
    }

    if (meetingPoint != -1) {
        vector<PathStep> forwardPath;
        int current = meetingPoint;
        if (meetingPoint != endActorId) {
            while (current != startActorId) {
                auto prev = forwardPrevious[current];
                forwardPath.insert(forwardPath.begin(), PathStep(
                    actors[current],
                    actors[prev.first],
                    prev.second
                ));
                current = prev.first;
            }
        }


        forwardPath.insert(forwardPath.begin(), PathStep(actors[startActorId]));
        vector<PathStep> backwardPath;
        current = meetingPoint;
        if (meetingPoint != startActorId) {
            while (current != endActorId) {
                auto prev = backwardPrevious[current];
                backwardPath.push_back(PathStep(
                    actors[prev.first],
                    actors[current],
                    prev.second
                ));
                current = prev.first;
            }
        }

        vector<PathStep> completePath = forwardPath;
        completePath.insert(completePath.end(), backwardPath.begin(), backwardPath.end());
        result.path = completePath;
    }

    auto endTime = chrono::high_resolution_clock::now();
    result.time_ms = chrono::duration<double, milli>(endTime - startTime).count();
    return result;
}