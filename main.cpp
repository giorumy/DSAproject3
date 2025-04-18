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
    string actorOneInput;
    string actorTwoInput;
    std::cout << "Input an actor's name: " << endl;
    std::getline(cin, actorOneInput);
    std::cout << "Input an actor to connect to: " << endl;
    std::getline(cin, actorTwoInput);


    vector<Movie> movies = data.getMovies(actorOneInput);
    vector<Movie> movies2 = data.getMovies(actorTwoInput);
    Actor input_actor = data.getActorObject(actorOneInput);
    Actor final_actor = data.getActorObject(actorTwoInput);
    graph.addActor(&input_actor, &final_actor);




    return 0;
}
