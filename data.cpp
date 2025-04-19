#include "data.h"

string Data::fetchData(const string& url) {
    // Create a command to fetch data using curl and save to a temporary file
    string tempFile = "temp_response.json";
    string command = "curl -s \"" + url + "\" > " + tempFile;

    cout << "Executing API request: " << url << endl; // Debug output

    // Execute the command
    int result = system(command.c_str());

    if (result != 0) {
        cerr << "Failed to execute command: " << command << endl;
        cerr << "Error code: " << result << endl;
        return "";
    }

    // Read the file
    ifstream file(tempFile);
    if (!file.is_open()) {
        cerr << "Failed to open temporary file: " << tempFile << endl;
        return "";
    }

    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    cout << "API Response length: " << content.length() << " bytes" << endl; // Debug output
    if (content.length() < 100) {
        cout << "Response content: " << content << endl; // Show short responses
    }

    // Save response for debugging (optional)
    ofstream debugFile("debug_response_" + to_string(rand()) + ".json");
    if (debugFile.is_open()) {
        debugFile << content;
        debugFile.close();
    }

    // Remove the temporary file
    remove(tempFile.c_str());

    return content;
}

string Data::urlEncode(string& str){
    string encoded;
    for(char c : str){
        if(c == ' '){
            encoded += "%20";
        } else if (c == '\'') {
            encoded += "%27"; // Encode apostrophes
        } else if (c == '&') {
            encoded += "%26"; // Encode ampersands
        } else if (c == ',') {
            encoded += "%2C"; // Encode commas
        } else {
            encoded += c;
        }
    }
    return encoded;
}

// Static method implementation
string Data::urlEncode(const string& str) {
    string encoded;
    for(char c : str){
        if(c == ' '){
            encoded += "%20";
        } else if (c == '\'') {
            encoded += "%27"; // Encode apostrophes
        } else if (c == '&') {
            encoded += "%26"; // Encode ampersands
        } else if (c == ',') {
            encoded += "%2C"; // Encode commas
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

    // Use system command to fetch data
    string response = fetchData(search_url);
    if (response.empty()) {
        cerr << "Failed to fetch actor data for " << actor_name << endl;
        return movies;
    }

    //parsing through JSON data
    try {
        json actor_data = json::parse(response);

        if (actor_data["results"].empty()) {
            std::cerr << "No actor found for: " << actor_name << "\n";
            return movies;
        }

        int actor_id = actor_data["results"][0]["id"];
        string actor_path = actor_data["results"][0]["profile_path"].is_null() ?
                           "" : actor_data["results"][0]["profile_path"].get<string>();

        actorID[actor_name] = actor_id;
        actorObjects[actor_id] = Actor(actor_id, actor_name, actor_path);

        ostringstream credits_url;
        credits_url << "https://api.themoviedb.org/3/person/" << actor_id << "/movie_credits?api_key=" << api_key;

        // Use system command to fetch data
        response = fetchData(credits_url.str());
        if (response.empty()) {
            cerr << "Failed to fetch movie credits for actor ID " << actor_id << endl;
            return movies;
        }

        json credits_data = json::parse(response);

        if (!credits_data.contains("cast") || credits_data["cast"].empty()) {
            std::cerr << "No movies found for actor: " << actor_name << "\n";
            return movies;
        }

        cout << "Found " << credits_data["cast"].size() << " movies for " << actor_name << endl;

        for (const auto& movie_json : credits_data["cast"]) {
            int id = movie_json.contains("id") ? movie_json["id"].get<int>() : -1;
            string title = movie_json.contains("title") ? movie_json["title"].get<string>() : "Unknown Title";
            string date = movie_json.contains("release_date") && !movie_json["release_date"].is_null() ?
                         movie_json["release_date"].get<string>() : "Unknown Date";
            string poster_path = movie_json.contains("poster_path") && !movie_json["poster_path"].is_null() ?
                                movie_json["poster_path"].get<string>() : ""; // Empty string if no profile path is found

            movies.emplace_back(id, title, date, poster_path);
        }

        cout << "Successfully processed " << movies.size() << " movies for " << actor_name << endl;
    } catch (const json::exception& e) {
        cerr << "JSON parsing error: " << e.what() << endl;
        cerr << "Failed to parse response for actor: " << actor_name << endl;
    } catch (const exception& e) {
        cerr << "Error processing actor data: " << e.what() << endl;
    }

    return movies;
}

vector<Actor> Data::getActors(string movie_name) {
    vector<Actor> actors = {};

    string encoded_name = urlEncode(movie_name);
    string search_url = "https://api.themoviedb.org/3/search/movie?api_key=" + api_key + "&query=" + encoded_name;

    // Use system command to fetch data
    string response = fetchData(search_url);
    if (response.empty()) {
        cerr << "Failed to fetch movie data for " << movie_name << endl;
        return actors;
    }

    try {
        json movie_data = json::parse(response);

        if (movie_data["results"].empty()) {
            std::cerr << "No movies found with title: " << movie_name << "\n";
            return actors;
        }

        int movie_id = movie_data["results"][0]["id"];

        ostringstream credits_url;
        credits_url << "https://api.themoviedb.org/3/movie/" << movie_id << "/credits?api_key=" << api_key;

        // Use system command to fetch data
        response = fetchData(credits_url.str());
        if (response.empty()) {
            cerr << "Failed to fetch movie credits for movie ID " << movie_id << endl;
            return actors;
        }

        json credits_data = json::parse(response);

        if (!credits_data.contains("cast") || credits_data["cast"].empty()) {
            std::cerr << "No cast data found for movie: " << movie_name << "\n";
            return actors;
        }

        cout << "Found " << credits_data["cast"].size() << " actors in " << movie_name << endl;

        for (const auto& actor_json : credits_data["cast"]) {
            int id = actor_json.contains("id") ? actor_json["id"].get<int>() : -1;
            string name = actor_json.contains("name") ? actor_json["name"].get<string>() : "Unknown Name";
            string profile_path = actor_json.contains("profile_path") && !actor_json["profile_path"].is_null() ?
                                actor_json["profile_path"].get<string>() : ""; // Empty string if no profile path is found

            // Add the actor to the actors vector
            actors.emplace_back(id, name, profile_path);
        }

        cout << "Successfully processed " << actors.size() << " actors for " << movie_name << endl;
    } catch (const json::exception& e) {
        cerr << "JSON parsing error: " << e.what() << endl;
        cerr << "Failed to parse response for movie: " << movie_name << endl;
    } catch (const exception& e) {
        cerr << "Error processing movie data: " << e.what() << endl;
    }

    return actors;
}