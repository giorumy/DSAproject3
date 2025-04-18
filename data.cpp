#include "data.h"

string Data::urlEncode(string& str){
  string encoded;
  for(char c : str){
    if(c == ' '){
      encoded += "%20";
    } else {
      encoded += c;
    }
  }
  return encoded;
}

Actor Data::getActorObject(string actor_name) {
  int id = actorID[actor_name];
  return actorObjects[id];
}
vector<Movie> Data::getMovies(string actor_name) {
  vector<Movie> movies = {};

  //getting movies
  string encoded_name = urlEncode(actor_name);
  string search_url = "https://api.themoviedb.org/3/search/person?api_key=" + api_key + "&query=" + encoded_name;
  string search_command = "curl -s \"" + search_url + "\" > actor_search.json";
  system(search_command.c_str());

  std::ifstream actor_file("actor_search.json");
  if (!actor_file.is_open()) {
    std::cerr << "Failed to open actor_search.json\n";
    return movies;
  }

  //parsing through JSON data
  json actor_data;
  actor_file >> actor_data;

  if (actor_data["results"].empty()) {
    std::cerr << "No actor found!\n";
    return movies;
  }

  int actor_id = actor_data["results"][0]["id"];
  string actor_path = actor_data["results"][0]["profile_path"];

  actorID[actor_name] = actor_id;
  actorObjects[actor_id] = Actor(actor_id, actor_name, actor_path);

  ostringstream credits_url;
  credits_url << "https://api.themoviedb.org/3/person/" << actor_id << "/movie_credits?api_key=" << api_key;
  string credits_command = "curl -s \"" + credits_url.str() + "\" > credits.json";
  system(credits_command.c_str());

  ifstream credits_file("credits.json");
  if (!credits_file.is_open()) {
    cerr << "Failed to open credits.json\n";
    return movies;
  }

  json credits_data;
  credits_file >> credits_data;

  for (const auto& movie_json : credits_data["cast"]) {
    int id = movie_json.contains("id") ? movie_json["id"].get<int>() : -1;
    string title = movie_json.contains("title") ? movie_json["title"].get<string>() : "Unknown Title";
    string date = movie_json.contains("release_date") ? movie_json["release_date"].get<string>() : "Unknown Date";
    string poster_path = movie_json.contains("poster_path") && !movie_json["poster_path"].is_null()
                          ? movie_json["poster_path"].get<string>(): ""; // Empty string if no profile path is found

    movies.emplace_back(id, title, date, poster_path);
  }

  return movies;
}

vector<Actor> Data::getActors(string movie_name) {
  vector<Actor> actors = {};

  string encoded_name = urlEncode(movie_name);
  string search_url = "https://api.themoviedb.org/3/search/movie?api_key=" + api_key + "&query=" + encoded_name;
  string search_command = "curl -s \"" + search_url + "\" > movie_search.json";
  system(search_command.c_str());

  std::ifstream movie_file("movie_search.json");
  if (!movie_file.is_open()) {
    std::cerr << "Failed to open movie_search.json\n";
    return actors;
  }

  json movie_data;
  movie_file >> movie_data;

  if (movie_data["results"].empty()) {
    std::cerr << "No movies found!\n";
    return actors;
  }

  int movie_id = movie_data["results"][0]["id"];

  ostringstream credits_url;
  credits_url << "https://api.themoviedb.org/3/movie/" << movie_id << "/credits?api_key=" << api_key;
  string credits_command = "curl -s \"" + credits_url.str() + "\" > credits.json";
  system(credits_command.c_str());

  ifstream credits_file("credits.json");
  if (!credits_file.is_open()) {
    cerr << "Failed to open credits.json\n";
    return actors;
  }

  json credits_data;
  credits_file >> credits_data;


  if (!credits_data.contains("cast")) {
    std::cerr << "No cast data found for movie.\n";
    return actors;
  }

  for (const auto& actor_json : credits_data["cast"]) {
    int id = actor_json.contains("id") ? actor_json["id"].get<int>() : -1;
    string name = actor_json.contains("name") ? actor_json["name"].get<string>() : "Unknown Name";
    string profile_path = actor_json.contains("profile_path") && !actor_json["profile_path"].is_null()
                          ? actor_json["profile_path"].get<string>()
                          : ""; // Empty string if no profile path is found

    // Add the actor to the actors vector
    actors.emplace_back(id, name, profile_path);
  }

  return actors;
}