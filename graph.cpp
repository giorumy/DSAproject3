#include "graph.h"

void Graph::addNode(Actor* actor) {
    actors[actor->id] = actor;
    adjacencyList[actor->id] = vector<Connection>();
}

void Graph::findSharedMovie(Actor *actor, int targetActorID) {
    vector<Movie> movies = API.getMovies(actor->name);

    bool found = false;

    for(Movie movie : movies) {
        vector<Actor> cast = API.getActors(movie.title);

        for(Actor person : cast) {
            if(person.id == targetActorID) {
                Movie* moviePtr = addMovie(movie.id, movie.title, movie.release_date, movie.poster_path);

                //add target actor if not already in graph
                if(actors.find(person.id) == actors.end()) {
                    Actor* targetActor = API.getActor(targetActorID);
                    addNode(targetActor);
                }

                //add connection
                addConnection(actor->id, person.id, moviePtr);
                found = true;
                break; //found a connection -> no need to check other actors in this movie
            }
        }
        if(found) break; //found a connection -> not need to check other movies
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