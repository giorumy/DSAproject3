#include <iostream>
#include "graph.h"

int main()
{
    std::cout << ":)" << std::endl;

    Data data("07663db07b6982f498aef71b6b0997f7");

    //example of API use:

    //with an actor:
    // string actor = "Tom Hanks";
    // vector<Movie> movies = data.getMovies(actor);
    // cout << "Movies featuring " << actor << ":\n";
    // for (const auto& movie : movies) {
    //     cout << "- " << movie.title << " (" << movie.release_date << "), ID: " << movie.id << ", Path: " << movie.poster_path << "\n";
    // }
    //
    // cout << "----------------------------------" << endl;
    //
    // //with a movie:
    // string movie = "Forrest Gump";
    // vector<Actor> actors = data.getActors(movie);
    // cout << "Actors featuring " << movie << ":\n";
    // for (const auto& actor : actors) {
    //     cout << "- " << actor.name << " (" << actor.id << "), Path: " << actor.profile_path << "\n";
    // }


    Graph graph;

    vector<Movie> movies = data.getMovies("Tom Hanks");
    Actor Tom = data.getActorObject("Tom Hanks");
    graph.addActor(&Tom);

    Actor Robin = data.getActorObject("Robin Wright");


    return 0;
}
